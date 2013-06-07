#ifndef CWSH_PATTERN_H
#define CWSH_PATTERN_H

class CwshPattern {
 public:
  CwshPattern(Cwsh *cwsh, const std::string &pattern="");

  bool expandWordToFiles(const CwshWord &word, CwshWordArray &words);

  bool expandPath(std::vector<std::string> &files);
  bool expandVar (std::vector<std::string> &names);

 private:
  CPtr<Cwsh>  cwsh_;
  std::string pattern_;
};

#endif
