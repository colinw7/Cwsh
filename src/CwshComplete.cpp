#include <CwshI.h>
#include <CFileMatch.h>
#include <CPathList.h>

namespace Cwsh {

Complete::
Complete(App *cwsh, const std::string &line) :
 cwsh_(cwsh), line_(line) {
}

bool
Complete::
complete(std::string &line1)
{
  line1 = "";

  std::string word;

  auto type = getCompletionType(&word);

  if (type == CompletionType::NONE)
    return false;

  std::string word1, word2;

  bool flag;

  if      (type == CompletionType::COMMAND)
    flag = completeCommand(word, word1, word2);
  else if (type == CompletionType::FILE)
    flag = completeFile(word, word1);
  else if (type == CompletionType::VAR)
    flag = completeVariable(word, word1);
  else if (type == CompletionType::USERS)
    flag = completeUsers(word, word1);
  else
    flag = false;

  uint len  = uint(word .size());
  uint len1 = uint(word1.size());

  if (! flag || len1 < len || word != word1.substr(0, len))
    return false;

  line1 = word1.substr(len);

  return true;
}

bool
Complete::
completeCommand(std::string &file, std::string &filePath)
{
  return completeCommand(line_, file, filePath);
}

bool
Complete::
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
Complete::
completeFile(std::string &file)
{
  return completeFile(line_, file);
}

bool
Complete::
completeFile(const std::string &file, std::string &file1)
{
  auto *fignore = cwsh_->lookupVariable("fignore");

  if (fignore) {
    int num_values = fignore->getNumValues();

    for (int i = 0; i < num_values; ++i)
      fileIgnoreList_.push_back("*." + fignore->getValue(i));
  }

  CFileMatch fileMatch;

  uint num = int(fileIgnoreList_.size());

  for (uint i = 0; i < num; ++i)
    fileMatch.addIgnorePattern(fileIgnoreList_[i]);

  file1 = fileMatch.mostMatchPrefix(file);

  if (file1.find(' ')) {
    std::string file2;

    uint len = uint(file1.size());

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
Complete::
completeExecFile(const std::string &file, std::string &file1)
{
  CFileMatch fileMatch;

  fileMatch.setOnlyExec();

  file1 = fileMatch.mostMatchPrefix(file);

  if (file1.find(' ')) {
    std::string file2;

    uint len = uint(file1.size());

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
Complete::
completeVariable(std::string &name)
{
  return completeVariable(line_, name);
}

bool
Complete::
completeVariable(const std::string &name, std::string &name1)
{
  std::string pattern_str = name + "*";

  std::vector<std::string> names;

  Pattern pattern(cwsh_, pattern_str);

  if (! pattern.expandVar(names))
    return false;

  name1 = CStrUtil::mostMatch(names);

  return true;
}

bool
Complete::
completeUsers(std::string &name)
{
  return completeUsers(line_, name);
}

bool
Complete::
completeUsers(const std::string &name, std::string &name1)
{
  std::string pattern_str = name + "*";

  std::vector<std::string> names;

  if (! String::matchUsers(pattern_str, names))
    return false;

  name1 = CStrUtil::mostMatch(names);

  return true;
}

CompletionType
Complete::
getCompletionType(std::string *word)
{
  auto *filec = cwsh_->lookupVariable("filec");

  if (! filec)
    return CompletionType::NONE;

  std::vector<std::string> words;

  String::addWords(line_, words);

  uint len = uint(line_.size());

  if (len > 0 && isspace(line_[len - 1]))
    words.push_back("");

  len = uint(words.size());

  if (len == 0)
    return CompletionType::NONE;

  const std::string &word1 = words[len - 1];

  uint len1 = uint(word1.size());

  if      (len1 > 0 && word1[0] == '$') {
    *word = word1.substr(1);

    return CompletionType::VAR;
  }
  else if (word1[0] == '~' && word1.find('/') == std::string::npos) {
    *word = word1.substr(1);

    return CompletionType::USERS;
  }
  else if (len == 1) {
    *word = word1;

    return CompletionType::COMMAND;
  }
  else {
    *word = word1;

    return CompletionType::FILE;
  }
}

}
