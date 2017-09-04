#include <CwshI.h>
#include <CGlob.h>

std::string
CwshString::
replaceHome(const std::string &str)
{
  std::string str1;

  if (! CFile::addTilde(str, str1))
    return str;

  return str1;
}

void
CwshString::
skipWordsToChar(const std::string &str, uint *i, int c)
{
  uint len = str.size();

  int brackets = 0;

  while (*i < len) {
    if      (str[*i] == '\"') {
      if (! CStrUtil::skipDoubleQuotedString(str, i))
        CWSH_THROW("Unmatched \".");
    }
    else if (str[*i] == '\'') {
      if (! CStrUtil::skipSingleQuotedString(str, i))
        CWSH_THROW("Unmatched \'.");
    }
    else if (str[*i] == '`') {
      if (! CStrUtil::skipBackQuotedString(str, i))
        CWSH_THROW("Unmatched `.");
    }
    else if (str[*i] == '(') {
      brackets++;

      (*i)++;
    }
    else if (str[*i] == ')') {
      if (brackets == 0) {
        if (c == str[*i])
          break;

        CWSH_THROW("Too many )'s.");
      }
      else
        brackets--;

      (*i)++;
    }
    else if (brackets == 0 && c == str[*i])
      break;
    else
      (*i)++;
  }

  if (*i >= len)
    CWSH_THROW(std::string("Unmatched ") + ((char) c) + ".");
}

void
CwshString::
skipWord(const std::string &str, uint *i)
{
  uint len = str.size();

  int brackets = 0;

  while (*i < len) {
    if      (str[*i] == '\"') {
      if (! CStrUtil::skipDoubleQuotedString(str, i))
        CWSH_THROW("Unmatched \".");
    }
    else if (str[*i] == '\'') {
      if (! CStrUtil::skipSingleQuotedString(str, i))
        CWSH_THROW("Unmatched \'.");
    }
    else if (str[*i] == '`') {
      if (! CStrUtil::skipBackQuotedString(str, i))
        CWSH_THROW("Unmatched `.");
    }
    else if (str[*i] == '(') {
      brackets++;

      (*i)++;
    }
    else if (str[*i] == ')') {
      if (brackets == 0)
        CWSH_THROW("Too many )'s.");
      else
        brackets--;

      (*i)++;
    }
    else if (brackets == 0 && isspace(str[*i]))
      break;
    else
      (*i)++;
  }
}

void
CwshString::
addWords(const std::string &str, std::vector<std::string> &words)
{
  uint len = str.size();

  std::string word;

  uint i = 0;

  while (i < len) {
    if      (isspace(str[i])) {
      if (word.size() > 0) {
        words.push_back(word);

        word = "";
      }

      CStrUtil::skipSpace(str, &i);
    }
    else if (str[i] == '\"') {
      uint i1 = i;

      if (! CStrUtil::skipDoubleQuotedString(str, &i))
        CWSH_THROW("Unmatched \".");

      word += str.substr(i1, i - i1);
    }
    else if (str[i] == '\'') {
      uint i1 = i;

      if (! CStrUtil::skipSingleQuotedString(str, &i))
        CWSH_THROW("Unmatched \'.");

      word += str.substr(i1, i - i1);
    }
    else if (str[i] == '`') {
      uint i1 = i;

      if (! CStrUtil::skipBackQuotedString(str, &i))
        CWSH_THROW("Unmatched `.");

      word += str.substr(i1, i - i1);
    }
    else if (str[i] == '(' || str[i] == ')' || str[i] == ';') {
      if (word.size() > 0) {
        words.push_back(word);

        word = "";
      }

      word += str[i++];

      words.push_back(word);

      word = "";
    }
    else if (str[i] == '&') {
      if (word.size() > 0) {
        words.push_back(word);

        word = "";
      }

      word += str[i++];

      if      (i < len && str[i] == '&')
        word += str[i++];

      words.push_back(word);

      word = "";
    }
    else if (str[i] == '|') {
      if (word.size() > 0) {
        words.push_back(word);

        word = "";
      }

      word += str[i++];

      if (i < len && (str[i] == '|' || str[i] == '&'))
        word += str[i++];

      words.push_back(word);

      word = "";
    }
    else if (str[i] == '<') {
      if (word.size() > 0) {
        words.push_back(word);

        word = "";
      }

      word += str[i++];

      if (i < len && str[i] == '<')
        word += str[i++];

      words.push_back(word);

      word = "";
    }
    else if (str[i] == '>') {
      if (word.size() > 0) {
        words.push_back(word);

        word = "";
      }

      word += str[i++];

      if      (i < len && str[i] == '!')
        word += str[i++];
      else if (i < len && str[i] == '>') {
        word += str[i++];

        if      (i < len && str[i] == '!')
          word += str[i++];
        else if (i < len && str[i] == '&') {
          word += str[i++];

          if (i < len && str[i] == '!')
            word += str[i++];
        }
      }
      else if (str[i] == '&') {
        word += str[i++];

        if (i < len && str[i] == '!')
          word += str[i++];
      }

      words.push_back(word);

      word = "";
    }
    else if (str[i] == ':') {
      if (CStrUtil::isIdentifier(word)) {
        words.push_back(word);

        word = str[i++];

        words.push_back(word);

        word = "";
      }
      else
        word += str[i++];
    }
    else if (str[i] == '\\') {
      word += str[i++];

      if (i < len)
        word += str[i++];
    }
    else
      word += str[i++];
  }

  if (word.size() > 0)
    words.push_back(word);
}

std::string
CwshString::
readLineFromFile(CFile *file)
{
  if (file->eof())
    CWSH_THROW("EOF");

  std::string line;

  file->readLine(line);

  while (line.size() > 0 && line[line.size() - 1] == '\\') {
    line = line.substr(0, line.size() - 1);

    std::string line1;

    file->readLine(line1);

    line += line1;
  }

  return line;
}

bool
CwshString::
matchUsers(const std::string &pattern, std::vector<std::string> &names)
{
  CGlob glob(pattern);

  glob.setAllowOr(false);
  glob.setAllowNonPrintable(true);

  std::vector<std::string> pw_names;

  COSUser::getUsers(pw_names);

  int num_names = pw_names.size();

  for (int i = 0; i < num_names; ++i) {
    if (glob.compare(pw_names[i]))
      names.push_back(pw_names[i]);
  }

  if (names.size() == 0)
    return false;

  return true;
}
