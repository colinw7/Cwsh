#include <CwshI.h>
#include <CFileMatch.h>
#include <CPathList.h>

CwshComplete::
CwshComplete(Cwsh *cwsh, const std::string &line) :
 cwsh_(cwsh), line_(line) {
}

bool
CwshComplete::
complete(std::string &line1)
{
  line1 = "";

  std::string word;

  CwshCompletionType type = getCompletionType(&word);

  if (type == CwshCompletionType::NONE)
    return false;

  std::string word1, word2;

  bool flag;

  if      (type == CwshCompletionType::COMMAND)
    flag = completeCommand(word, word1, word2);
  else if (type == CwshCompletionType::FILE)
    flag = completeFile(word, word1);
  else if (type == CwshCompletionType::VAR)
    flag = completeVariable(word, word1);
  else if (type == CwshCompletionType::USERS)
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
completeCommand(std::string &file, std::string &filePath)
{
  return completeCommand(line_, file, filePath);
}

bool
CwshComplete::
completeCommand(const std::string &path, std::string &file, std::string &filePath)
{
  filePath = "";

  auto pos = path.rfind('/');

  if (pos == std::string::npos) {
    CPathList pathList;

    pathList.addEnvValue("PATH");

    file = pathList.mostMatchPrefix(path, filePath);
  }
  else {
    if (! completeExecFile(path, file))
      return false;
  }

  return true;
}

bool
CwshComplete::
completeFile(std::string &file)
{
  return completeFile(line_, file);
}

bool
CwshComplete::
completeFile(const std::string &file, std::string &file1)
{
  CwshVariable *fignore = cwsh_->lookupVariable("fignore");

  if (fignore) {
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
    std::string file2;

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
completeExecFile(const std::string &file, std::string &file1)
{
  CFileMatch fileMatch;

  fileMatch.setOnlyExec();

  file1 = fileMatch.mostMatchPrefix(file);

  if (file1.find(' ')) {
    std::string file2;

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
completeVariable(std::string &name)
{
  return completeVariable(line_, name);
}

bool
CwshComplete::
completeVariable(const std::string &name, std::string &name1)
{
  std::string pattern_str = name + "*";

  std::vector<std::string> names;

  CwshPattern pattern(cwsh_, pattern_str);

  if (! pattern.expandVar(names))
    return false;

  name1 = CStrUtil::mostMatch(names);

  return true;
}

bool
CwshComplete::
completeUsers(std::string &name)
{
  return completeUsers(line_, name);
}

bool
CwshComplete::
completeUsers(const std::string &name, std::string &name1)
{
  std::string pattern_str = name + "*";

  std::vector<std::string> names;

  if (! CwshString::matchUsers(pattern_str, names))
    return false;

  name1 = CStrUtil::mostMatch(names);

  return true;
}

CwshCompletionType
CwshComplete::
getCompletionType(std::string *word)
{
  CwshVariable *filec = cwsh_->lookupVariable("filec");

  if (! filec)
    return CwshCompletionType::NONE;

  std::vector<std::string> words;

  CwshString::addWords(line_, words);

  uint len = line_.size();

  if (len > 0 && isspace(line_[len - 1]))
    words.push_back("");

  len = words.size();

  if (len == 0)
    return CwshCompletionType::NONE;

  const std::string &word1 = words[len - 1];

  uint len1 = word1.size();

  if      (len1 > 0 && word1[0] == '$') {
    *word = word1.substr(1);

    return CwshCompletionType::VAR;
  }
  else if (word1[0] == '~' && word1.find('/') == std::string::npos) {
    *word = word1.substr(1);

    return CwshCompletionType::USERS;
  }
  else if (len == 1) {
    *word = word1;

    return CwshCompletionType::COMMAND;
  }
  else {
    *word = word1;

    return CwshCompletionType::FILE;
  }
}
