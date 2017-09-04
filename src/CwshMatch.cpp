#include <CwshI.h>
#include <CFileMatch.h>
#include <COSTerm.h>

CwshMatch::
CwshMatch(Cwsh *cwsh) :
 cwsh_(cwsh) {
}

bool
CwshMatch::
showMatch(const std::string &line)
{
  std::string word;

  CwshComplete complete(cwsh_, line);

  CwshCompletionType type = complete.getCompletionType(&word);

  if (type == CwshCompletionType::NONE) {
    cwsh_->beep();

    return false;
  }

  word += "*";

  std::vector<std::string> words;

  if      (type == CwshCompletionType::COMMAND)
    getPathMatch(word, words);
  else if (type == CwshCompletionType::FILE) {
    std::string::size_type pos = word.rfind('/');

    if (pos != std::string::npos) {
      std::string lhs = word.substr(0, pos);
      std::string rhs = word.substr(pos + 1);

      CDir::enter(lhs);

      getFileMatch(rhs, words);

      CDir::leave();
    }
    else
      getFileMatch(word, words);
  }
  else if (type == CwshCompletionType::VAR)
    getVarMatch(word, words);
  else if (type == CwshCompletionType::USERS)
    getUsersMatch(word, words);
  else
    return false;

  CStrUtil::sort(words);

  std::vector<std::string> uniq_words;

  CStrUtil::uniq(words, uniq_words);

  std::cout << std::endl;

  print(uniq_words);

  return true;
}

bool
CwshMatch::
getPathMatch(const std::string &pattern_str, std::vector<std::string> &words)
{
  CwshPattern pattern(cwsh_, pattern_str);

  if (! pattern.expandPath(words)) {
    cwsh_->beep();

    return false;
  }

  return true;
}

bool
CwshMatch::
getFileMatch(const std::string &pattern_str, std::vector<std::string> &words)
{
  CFileMatch fileMatch;

  if (! fileMatch.matchPrefix(pattern_str, words)) {
    cwsh_->beep();

    return false;
  }

  return true;
}

bool
CwshMatch::
getVarMatch(const std::string &pattern_str, std::vector<std::string> &words)
{
  CwshPattern pattern(cwsh_, pattern_str);

  if (! pattern.expandVar(words)) {
    cwsh_->beep();

    return false;
  }

  return true;
}

bool
CwshMatch::
getUsersMatch(const std::string &pattern_str, std::vector<std::string> &words)
{
  if (! CwshString::matchUsers(pattern_str, words)) {
    cwsh_->beep();

    return false;
  }

  return true;
}

void
CwshMatch::
print(std::vector<std::string> &words)
{
  int max_len = CStrUtil::maxLen(words);

  int screen_width, screen_height;

  COSTerm::getCharSize(&screen_width, &screen_height);

  int num_words = words.size();

  int words_per_line = std::max(screen_width / (max_len + 1), 1);

  int num_lines = num_words / words_per_line;

  if ((num_words % words_per_line) != 0)
    ++num_lines;

  int i = 0;
  int j = 0;

  while (i < num_words && j < num_lines) {
    int len = words[i].size();

    std::cout << words[i];

    for (int k = 0; k <= max_len - len; ++k)
      std::cout << " ";

    i += num_lines;

    if (i >= num_words) {
      std::cout << std::endl;

      ++j;

      i = j;
    }
  }
}
