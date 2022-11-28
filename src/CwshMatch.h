#ifndef CWSH_MATCH_H
#define CWSH_MATCH_H

namespace Cwsh {

class Match {
 public:
  Match(App *cwsh);

  bool showMatch(const std::string &line);

 private:
  bool getPathMatch (const std::string &pattern, std::vector<std::string> &words);
  bool getFileMatch (const std::string &pattern, std::vector<std::string> &words);
  bool getVarMatch  (const std::string &pattern, std::vector<std::string> &words);
  bool getUsersMatch(const std::string &pattern, std::vector<std::string> &words);

  void print(const std::vector<std::string> &words) const;

  bool matchUsers(const std::string &pattern, std::vector<std::string> &names);

 private:
  CPtr<App> cwsh_;
};

}

#endif
