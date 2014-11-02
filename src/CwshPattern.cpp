#include <CwshI.h>
#include <CFileMatch.h>
#include <CPathList.h>
#include <unistd.h>

CwshPattern::
CwshPattern(Cwsh *cwsh, const string &pattern) :
 cwsh_(cwsh), pattern_(pattern)
{
}

bool
CwshPattern::
expandWordToFiles(const CwshWord &word, CwshWordArray &words)
{
  CwshVariable *variable = cwsh_->lookupVariable("noglob");

  if (variable != NULL)
    return false;

  //------

  const CwshSubWordArray &sub_words = word.getSubWords();

  string word1;

  int num_sub_words = sub_words.size();

  for (int i = 0; i < num_sub_words; i++) {
    CwshSubWordType type = sub_words[i].getType();

    if      (type == CWSH_SUB_WORD_TYPE_BACK_QUOTED)
      word1 += sub_words[i].getWord();
    else if (type == CWSH_SUB_WORD_TYPE_DOUBLE_QUOTED ||
             type == CWSH_SUB_WORD_TYPE_SINGLE_QUOTED) {
      string word2 = CStrUtil::addEscapeChars(sub_words[i].getWord(), "*?[]");

      word1 += word2;
    }
    else
      word1 += sub_words[i].getWord();
  }

  CwshWildCard wildcard(word1);

  if (! wildcard.isValid())
    return false;

  vector<string> words1;

  CFileMatch fileMatch;

  if (! fileMatch.matchPattern(word.getWord(), words1))
    return false;

  if (words1.size() == 0) {
    CwshVariable *variable = cwsh_->lookupVariable("nonomatch");

    if (variable == NULL)
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
expandPath(vector<string> &files)
{
  CPathList pathList;

  pathList.addEnvValue("PATH");

  vector<string> dirs;

  return pathList.matchPattern(pattern_, dirs, files);
}

bool
CwshPattern::
expandVar(vector<string> &names)
{
  CwshWildCard compile(pattern_);

  CwshVariableList::iterator pvariable1 = cwsh_->variablesBegin();
  CwshVariableList::iterator pvariable2 = cwsh_->variablesEnd  ();

  for ( ; pvariable1 != pvariable2; ++pvariable1) {
    if (compile.checkMatch((*pvariable1)->getName()))
      names.push_back((*pvariable1)->getName());
  }

  vector<string> env_names;
  vector<string> env_values;

  CEnvInst.getNameValues(env_names, env_values);

  int num_env_names = env_names.size();

  for (int i = 0; i < num_env_names; i++) {
    if (compile.checkMatch(env_names[i]))
      names.push_back(env_names[i]);
  }

  return true;
}
