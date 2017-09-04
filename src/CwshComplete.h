#ifndef CWSH_COMPLETE_H
#define CWSH_COMPLETE_H

enum class CwshCompletionType {
  NONE,
  COMMAND,
  FILE,
  VAR,
  USERS,
};

class CwshComplete {
 public:
  CwshComplete(Cwsh *cwsh, const std::string &line);

  bool complete(std::string &line1);

  CwshCompletionType getCompletionType(std::string *word);

  bool completeCommand (std::string &file, std::string &filePath);
  bool completeFile    (std::string &file);
  bool completeVariable(std::string &name);
  bool completeUsers   (std::string &name);

 private:
  bool completeCommand (const std::string &path, std::string &file, std::string &filePath);
  bool completeFile    (const std::string &file, std::string &file1);
  bool completeExecFile(const std::string &file, std::string &file1);
  bool completeVariable(const std::string &name, std::string &name1);
  bool completeUsers   (const std::string &name, std::string &name1);

  bool matchUsers(const std::string &pattern, std::vector<std::string> &names);

 private:
  typedef std::vector<std::string> StringArray;

  CPtr<Cwsh>  cwsh_;
  std::string line_;
  StringArray file_ignore_list_;
};

#endif
