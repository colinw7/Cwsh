#ifndef CWSH_EXPR_PARSE_H
#define CWSH_EXPR_PARSE_H

class CwshExprParse {
 public:
  CwshExprParse(Cwsh *cwsh);

  std::string parse(const std::string &str, uint *pos);
  std::string read (const std::string &str, uint *pos);

  void stack(CwshExprStackStack *stack, const std::string &expression);

 private:
  void subStack(CwshExprStackStack *stack, const std::string &str, uint *pos);

  CwshExprOperator *readFileOperator  (const std::string &str, uint *pos);
  CwshExprOperator *readUnaryOperator (const std::string &str, uint *pos);
  CwshExprOperator *readBinaryOperator(const std::string &str, uint *pos);

  std::string readInteger(const std::string &str, uint *pos);
  std::string readString (const std::string &str, uint *pos);

  bool skipToCloseBracket(const std::string &str, uint *pos);

 private:
  static std::string unary_operator_chars_;
  static std::string binary_operator_chars_;
  static std::string file_operator_chars_;

  CPtr<Cwsh> cwsh_;
};

#endif
