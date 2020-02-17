#include <CwshI.h>
#include <CFileMatch.h>
#include <CPathList.h>

CwshPattern::
CwshPattern(Cwsh *cwsh, const std::string &pattern) :
 cwsh_(cwsh), pattern_(pattern)
{
}

bool
CwshPattern::
expandWordToFiles(const CwshWord &word, CwshWordArray &words)
{
  CwshVariable *variable = cwsh_->lookupVariable("noglob");

  if (variable)
    return false;

  //------

  const CwshSubWordArray &sub_words = word.getSubWords();

  std::string word1;

  int num_sub_words = sub_words.size();

  for (int i = 0; i < num_sub_words; i++) {
    CwshSubWordType type = sub_words[i].getType();

    if      (type == CwshSubWordType::BACK_QUOTED)
      word1 += sub_words[i].getWord();
    else if (type == CwshSubWordType::DOUBLE_QUOTED ||
             type == CwshSubWordType::SINGLE_QUOTED) {
      std::string word2 = CStrUtil::addEscapeChars(sub_words[i].getWord(), "*?[]");

      word1 += word2;
    }
    else
      word1 += sub_words[i].getWord();
  }

  CwshWildCard wildcard(word1);

  if (! wildcard.isValid())
    return false;

  std::vector<std::string> words1;

  CFileMatch fileMatch;

  if (! fileMatch.matchPattern(word.getWord(), words1))
    return false;

  if (words1.size() == 0) {
    CwshVariable *variable1 = cwsh_->lookupVariable("nonomatch");

    if (! variable1)
      CWSH_THROW("No match.");

    return false;
  }

  CStrUtil::sort(words1);

  int num_words1 = words1.size();

  for (int i = 0; i < num_words1; i++)
    words.push_back(CwshWord(words1[i]));

  return true;
}

bool
CwshPattern::
expandPath(std::vector<std::string> &files)
{
  CPathList pathList;

  pathList.addEnvValue("PATH");

  std::vector<std::string> dirs;

  return pathList.matchPattern(pattern_, dirs, files);
}

bool
CwshPattern::
expandVar(std::vector<std::string> &names)
{
  CwshWildCard compile(pattern_);

  auto pvariable1 = cwsh_->variablesBegin();
  auto pvariable2 = cwsh_->variablesEnd  ();

  for ( ; pvariable1 != pvariable2; ++pvariable1) {
    if (compile.checkMatch((*pvariable1)->getName()))
      names.push_back((*pvariable1)->getName());
  }

  if (! CEnvInst.matchPattern(pattern_, names))
    return false;

  return true;
}
