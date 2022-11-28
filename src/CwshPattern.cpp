#include <CwshI.h>
#include <CFileMatch.h>
#include <CPathList.h>

namespace Cwsh {

Pattern::
Pattern(App *cwsh, const std::string &pattern) :
 cwsh_(cwsh), pattern_(pattern)
{
}

bool
Pattern::
expandWordToFiles(const Word &word, WordArray &words)
{
  auto *variable = cwsh_->lookupVariable("noglob");

  if (variable)
    return false;

  //------

  const auto &sub_words = word.getSubWords();

  std::string word1;

  auto num_sub_words = sub_words.size();

  for (size_t i = 0; i < num_sub_words; i++) {
    auto type = sub_words[i].getType();

    if      (type == SubWordType::BACK_QUOTED)
      word1 += sub_words[i].getWord();
    else if (type == SubWordType::DOUBLE_QUOTED || type == SubWordType::SINGLE_QUOTED) {
      std::string word2 = CStrUtil::addEscapeChars(sub_words[i].getWord(), "*?[]");

      word1 += word2;
    }
    else
      word1 += sub_words[i].getWord();
  }

  WildCard wildcard(word1);

  if (! wildcard.isValid())
    return false;

  std::vector<std::string> words1;

  CFileMatch fileMatch;

  if (! fileMatch.matchPattern(word.getWord(), words1))
    return false;

  if (words1.size() == 0) {
    auto *variable1 = cwsh_->lookupVariable("nonomatch");

    if (! variable1)
      CWSH_THROW("No match.");

    return false;
  }

  CStrUtil::sort(words1);

  for (const auto &word1 : words1)
    words.push_back(Word(word1));

  return true;
}

bool
Pattern::
expandPath(std::vector<std::string> &files)
{
  CPathList pathList;

  pathList.addEnvValue("PATH");

  std::vector<std::string> dirs;

  return pathList.matchPattern(pattern_, dirs, files);
}

bool
Pattern::
expandVar(std::vector<std::string> &names)
{
  WildCard compile(pattern_);

  for (auto *var : cwsh_->variables()) {
    if (compile.checkMatch(var->getName()))
      names.push_back(var->getName());
  }

  if (! CEnvInst.matchPattern(pattern_, names))
    return false;

  return true;
}

}
