#ifndef CWSH_EXPR_OPERATOR_H
#define CWSH_EXPR_OPERATOR_H

enum CwshExprOperatorType {
  CWSH_EXPR_OPERATOR_TYPE_OPEN_BRACKET,
  CWSH_EXPR_OPERATOR_TYPE_CLOSE_BRACKET,

  CWSH_EXPR_OPERATOR_TYPE_UNARY_PLUS,
  CWSH_EXPR_OPERATOR_TYPE_UNARY_MINUS,
  CWSH_EXPR_OPERATOR_TYPE_LOGICAL_NOT,
  CWSH_EXPR_OPERATOR_TYPE_BIT_NOT,
  CWSH_EXPR_OPERATOR_TYPE_IS_DIRECTORY,
  CWSH_EXPR_OPERATOR_TYPE_IS_FILE,
  CWSH_EXPR_OPERATOR_TYPE_IS_PLAIN,
  CWSH_EXPR_OPERATOR_TYPE_IS_OWNER,
  CWSH_EXPR_OPERATOR_TYPE_IS_READABLE,
  CWSH_EXPR_OPERATOR_TYPE_IS_WRITABLE,
  CWSH_EXPR_OPERATOR_TYPE_IS_EXECUTABLE,
  CWSH_EXPR_OPERATOR_TYPE_IS_ZERO,

  CWSH_EXPR_OPERATOR_TYPE_PLUS,
  CWSH_EXPR_OPERATOR_TYPE_MINUS,
  CWSH_EXPR_OPERATOR_TYPE_TIMES,
  CWSH_EXPR_OPERATOR_TYPE_DIVIDE,
  CWSH_EXPR_OPERATOR_TYPE_MODULUS,
  CWSH_EXPR_OPERATOR_TYPE_LESS,
  CWSH_EXPR_OPERATOR_TYPE_LESS_OR_EQUAL,
  CWSH_EXPR_OPERATOR_TYPE_GREATER,
  CWSH_EXPR_OPERATOR_TYPE_GREATER_OR_EQUAL,
  CWSH_EXPR_OPERATOR_TYPE_EQUAL,
  CWSH_EXPR_OPERATOR_TYPE_NOT_EQUAL,
  CWSH_EXPR_OPERATOR_TYPE_MATCH_EQUAL,
  CWSH_EXPR_OPERATOR_TYPE_NO_MATCH_EQUAL,
  CWSH_EXPR_OPERATOR_TYPE_LOGICAL_AND,
  CWSH_EXPR_OPERATOR_TYPE_LOGICAL_OR,
  CWSH_EXPR_OPERATOR_TYPE_BIT_AND,
  CWSH_EXPR_OPERATOR_TYPE_BIT_OR,
  CWSH_EXPR_OPERATOR_TYPE_BIT_XOR,
  CWSH_EXPR_OPERATOR_TYPE_BIT_LSHIFT,
  CWSH_EXPR_OPERATOR_TYPE_BIT_RSHIFT,
};

class CwshExprOperator {
 public:
  static CwshExprOperator *lookup(CwshExprOperatorType type);

  CwshExprOperatorType  getType      () const { return type_; }
  const std::string    &getToken     () const { return token_; }
  int                   getPrecedence() const { return precedence_; }

  bool doesAssociateLtoR() const { return associate_l_to_r_; }
  bool isPunctuation    () const { return punctuation_; }
  bool isUnary          () const { return unary_; }
  bool isBinary         () const { return (! unary_ && ! punctuation_); }

 private:
  CwshExprOperator(CwshExprOperatorType type, const std::string &token, uint precedence,
                   bool associate_ltor, bool punctuation, bool unary);

 private:
  static CwshExprOperator operators_[];
  static int              num_operators_;

  CwshExprOperatorType type_;
  std::string          token_;
  uint                 precedence_;
  bool                 associate_l_to_r_;
  bool                 punctuation_;
  bool                 unary_;
};

#endif
