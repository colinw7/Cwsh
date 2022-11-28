#ifndef CWSH_PATTERN_H
#define CWSH_PATTERN_H

namespace Cwsh {

class Pattern {
 public:
  Pattern(App *cwsh, const std::string &pattern="");

  bool expandWordToFiles(const Word &word, WordArray &words);

  bool expandPath(std::vector<std::string> &files);
  bool expandVar (std::vector<std::string> &names);

 private:
  CPtr<App>   cwsh_;
  std::string pattern_;
};

}

#endif
