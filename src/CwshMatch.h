#ifndef CWSH_MATCH_H
#define CWSH_MATCH_H

class CwshMatch {
 public:
  CwshMatch(Cwsh *cwsh);

  bool showMatch(const std::string &line);

 private:
  bool getPathMatch (const std::string &pattern, std::vector<std::string> &words);
  bool getFileMatch (const std::string &pattern, std::vector<std::string> &words);
  bool getVarMatch  (const std::string &pattern, std::vector<std::string> &words);
  bool getUsersMatch(const std::string &pattern, std::vector<std::string> &words);

  void print(std::vector<std::string> &words);

  bool matchUsers(const std::string &pattern, std::vector<std::string> &names);

 private:
  CPtr<Cwsh> cwsh_;
};

#endif
