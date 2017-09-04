#ifndef CWSH_EXPR_OPERATOR_H
#define CWSH_EXPR_OPERATOR_H

enum class CwshExprOperatorType {
  OPEN_BRACKET,
  CLOSE_BRACKET,

  UNARY_PLUS,
  UNARY_MINUS,
  LOGICAL_NOT,
  BIT_NOT,
  IS_DIRECTORY,
  IS_FILE,
  IS_PLAIN,
  IS_OWNER,
  IS_READABLE,
  IS_WRITABLE,
  IS_EXECUTABLE,
  IS_ZERO,

  PLUS,
  MINUS,
  TIMES,
  DIVIDE,
  MODULUS,
  LESS,
  LESS_OR_EQUAL,
  GREATER,
  GREATER_OR_EQUAL,
  EQUAL,
  NOT_EQUAL,
  MATCH_EQUAL,
  NO_MATCH_EQUAL,
  LOGICAL_AND,
  LOGICAL_OR,
  BIT_AND,
  BIT_OR,
  BIT_XOR,
  BIT_LSHIFT,
  BIT_RSHIFT,
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
