#ifndef CWSH_EXPR_PARSE_H
#define CWSH_EXPR_PARSE_H

namespace Cwsh {

class ExprParse {
 public:
  ExprParse(App *cwsh);

  std::string parse(const std::string &str, uint *pos);
  std::string read (const std::string &str, uint *pos);

  void stack(ExprStackStack *stack, const std::string &expression);

 private:
  void subStack(ExprStackStack *stack, const std::string &str, uint *pos);

  ExprOperator *readFileOperator  (const std::string &str, uint *pos);
  ExprOperator *readUnaryOperator (const std::string &str, uint *pos);
  ExprOperator *readBinaryOperator(const std::string &str, uint *pos);

  std::string readInteger(const std::string &str, uint *pos);
  std::string readString (const std::string &str, uint *pos);

  bool skipToCloseBracket(const std::string &str, uint *pos);

 private:
  static std::string unaryOperatorChars_;
  static std::string binaryOperatorChars_;
  static std::string fileOperatorChars_;

  CPtr<App> cwsh_;
};

}

#endif
