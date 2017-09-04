#ifndef CWSH_HISTORY_PARSER_H
#define CWSH_HISTORY_PARSER_H

class CwshHistoryParser {
 public:
  CwshHistoryParser(Cwsh *cwsh);
 ~CwshHistoryParser();

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
  CPtr<Cwsh>                 cwsh_;
  std::string                str_;
  uint                       pos_       { 0 };
  CwshHistoryOperation      *operation_ { nullptr };
  CwshHistoryOperationArray  operations_;
  bool                       print_     { false };
};

#endif
