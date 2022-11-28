#ifndef CWSH_HISTORY_PARSER_H
#define CWSH_HISTORY_PARSER_H

namespace Cwsh {

class HistoryParser {
 public:
  HistoryParser(App *cwsh);
 ~HistoryParser();

  std::string parseLine(const std::string &str);

  void parse(const std::string &str);

  std::string apply();
  std::string apply(const std::vector<std::string> &words);

  bool getPrint() const { return print_; }
  void setPrint(bool print) { print_ = print; }

  void display() const;

 private:
  bool isCommand();
  void parseCommand();
  void parseSubStr();
  void parseArgSelector();
  bool parseModifier();
  void parseQuickSubStr();
  bool isSubStrChar(char c);

 private:
  CPtr<App>              cwsh_;
  std::string            str_;
  uint                   pos_       { 0 };
  HistoryOperation      *operation_ { nullptr };
  HistoryOperationArray  operations_;
  bool                   print_     { false };
};

}

#endif
