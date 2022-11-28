#include <CwshI.h>
#include <CFileMatch.h>
#include <COSTerm.h>

namespace Cwsh {

Match::
Match(App *cwsh) :
 cwsh_(cwsh) {
}

bool
Match::
showMatch(const std::string &line)
{
  std::string word;

  Complete complete(cwsh_, line);

  auto type = complete.getCompletionType(&word);

  if (type == CompletionType::NONE) {
    cwsh_->beep();

    return false;
  }

  word += "*";

  std::vector<std::string> words;

  if      (type == CompletionType::COMMAND)
    getPathMatch(word, words);
  else if (type == CompletionType::FILE) {
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
  else if (type == CompletionType::VAR)
    getVarMatch(word, words);
  else if (type == CompletionType::USERS)
    getUsersMatch(word, words);
  else
    return false;

  CStrUtil::sort(words);

  std::vector<std::string> uniqWords;

  CStrUtil::uniq(words, uniqWords);

  std::cout << "\n";

  print(uniqWords);

  return true;
}

bool
Match::
getPathMatch(const std::string &patternStr, std::vector<std::string> &words)
{
  Pattern pattern(cwsh_, patternStr);

  if (! pattern.expandPath(words)) {
    cwsh_->beep();

    return false;
  }

  return true;
}

bool
Match::
getFileMatch(const std::string &patternStr, std::vector<std::string> &words)
{
  CFileMatch fileMatch;

  if (! fileMatch.matchPrefix(patternStr, words)) {
    cwsh_->beep();

    return false;
  }

  return true;
}

bool
Match::
getVarMatch(const std::string &patternStr, std::vector<std::string> &words)
{
  Pattern pattern(cwsh_, patternStr);

  if (! pattern.expandVar(words)) {
    cwsh_->beep();

    return false;
  }

  return true;
}

bool
Match::
getUsersMatch(const std::string &patternStr, std::vector<std::string> &words)
{
  if (! String::matchUsers(patternStr, words)) {
    cwsh_->beep();

    return false;
  }

  return true;
}

void
Match::
print(const std::vector<std::string> &words) const
{
  int maxLen = CStrUtil::maxLen(words);

  int screenRows, screenCols;

  COSTerm::getCharSize(&screenRows, &screenCols);

  int numWords = int(words.size());

  int wordsPerLine = std::max(screenCols/(maxLen + 1), 1);

  int numLines = numWords / wordsPerLine;

  if ((numWords % wordsPerLine) != 0)
    ++numLines;

  int i = 0;
  int j = 0;

  while (i < numWords && j < numLines) {
    int len = int(words[i].size());

    std::cout << words[i];

    for (int k = 0; k <= maxLen - len; ++k)
      std::cout << " ";

    i += numLines;

    if (i >= numWords) {
      std::cout << "\n";

      ++j;

      i = j;
    }
  }
}

}
