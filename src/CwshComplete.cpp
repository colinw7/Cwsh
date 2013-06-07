#include "CwshI.h"
#include <CFileMatch.h>
#include <CPathList.h>

CwshComplete::
CwshComplete(Cwsh *cwsh, const string &line) :
 cwsh_(cwsh), line_(line) {
}

bool
CwshComplete::
complete(string &line1)
{
  line1 = "";

  string word;

  CwshCompletionType type = getCompletionType(&word);

  if (type == CWSH_COMPLETION_TYPE_NONE)
    return false;

  string word1;

  bool flag;

  if      (type == CWSH_COMPLETION_TYPE_COMMAND)
    flag = completeCommand(word, word1);
  else if (type == CWSH_COMPLETION_TYPE_FILE)
    flag = completeFile(word, word1);
  else if (type == CWSH_COMPLETION_TYPE_VAR)
    flag = completeVariable(word, word1);
  else if (type == CWSH_COMPLETION_TYPE_USERS)
    flag = completeUsers(word, word1);
  else
    flag = false;

  uint len  = word .size();
  uint len1 = word1.size();

  if (! flag || len1 < len || word != word1.substr(0, len))
    return false;

  line1 = word1.substr(len);

  return true;
}

bool
CwshComplete::
completeCommand(string &path)
{
  return completeCommand(line_, path);
}

bool
CwshComplete::
completeCommand(const string &path, string &path1)
{
  string::size_type pos = path.rfind('/');

  if (pos == string::npos) {
    CPathList pathList;

    pathList.addEnvValue("PATH");

    path1 = pathList.mostMatchPrefix(path);
  }
  else {
    if (! completeExecFile(path, path1))
      return false;
  }

  return true;
}

bool
CwshComplete::
completeFile(string &file)
{
  return completeFile(line_, file);
}

bool
CwshComplete::
completeFile(const string &file, string &file1)
{
  CwshVariable *fignore = cwsh_->lookupVariable("fignore");

  if (fignore != NULL) {
    int num_values = fignore->getNumValues();

    for (int i = 0; i < num_values; ++i)
      file_ignore_list_.push_back("*." + fignore->getValue(i));
  }

  CFileMatch fileMatch;

  uint num = file_ignore_list_.size();

  for (uint i = 0; i < num; ++i)
    fileMatch.addIgnorePattern(file_ignore_list_[i]);

  file1 = fileMatch.mostMatchPrefix(file);

  if (file1.find(' ')) {
    string file2;

    uint len = file1.size();

    for (uint i = 0; i < len; ++i) {
      if (file1[i] == ' ')
        file2 += "\\";

      file2 += file1[i];
    }

    file1 = file2;
  }

  return true;
}

bool
CwshComplete::
completeExecFile(const string &file, string &file1)
{
  CFileMatch fileMatch;

  fileMatch.setOnlyExec();

  file1 = fileMatch.mostMatchPrefix(file);

  if (file1.find(' ')) {
    string file2;

    uint len = file1.size();

    for (uint i = 0; i < len; ++i) {
      if (file1[i] == ' ')
        file2 += "\\";

      file2 += file1[i];
    }

    file1 = file2;
  }

  return true;
}

bool
CwshComplete::
completeVariable(string &name)
{
  return completeVariable(line_, name);
}

bool
CwshComplete::
completeVariable(const string &name, string &name1)
{
  string pattern_str = name + "*";

  vector<string> names;

  CwshPattern pattern(cwsh_, pattern_str);

  if (! pattern.expandVar(names))
    return false;

  name1 = CStrUtil::mostMatch(names);

  return true;
}

bool
CwshComplete::
completeUsers(string &name)
{
  return completeUsers(line_, name);
}

bool
CwshComplete::
completeUsers(const string &name, string &name1)
{
  string pattern_str = name + "*";

  vector<string> names;

  if (! CwshString::matchUsers(pattern_str, names))
    return false;

  name1 = CStrUtil::mostMatch(names);

  return true;
}

CwshCompletionType
CwshComplete::
getCompletionType(string *word)
{
  CwshVariable *filec = cwsh_->lookupVariable("filec");

  if (filec == NULL)
    return CWSH_COMPLETION_TYPE_NONE;

  vector<string> words;

  CwshString::addWords(line_, words);

  uint len = line_.size();

  if (len > 0 && isspace(line_[len - 1]))
    words.push_back("");

  len = words.size();

  if (len == 0)
    return CWSH_COMPLETION_TYPE_NONE;

  const string &word1 = words[len - 1];

  uint len1 = word1.size();

  if      (len1 > 0 && word1[0] == '$') {
    *word = word1.substr(1);

    return CWSH_COMPLETION_TYPE_VAR;
  }
  else if (word1[0] == '~' && word1.find('/') == string::npos) {
    *word = word1.substr(1);

    return CWSH_COMPLETION_TYPE_USERS;
  }
  else if (len == 1) {
    *word = word1;

    return CWSH_COMPLETION_TYPE_COMMAND;
  }
  else {
    *word = word1;

    return CWSH_COMPLETION_TYPE_FILE;
  }
}
