#ifndef CWSH_EXPR_OPERATOR_H
#define CWSH_EXPR_OPERATOR_H

namespace Cwsh {

enum class ExprOperatorType {
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

class ExprOperator {
 public:
  static ExprOperator *lookup(ExprOperatorType type);

  ExprOperatorType   getType      () const { return type_; }
  const std::string &getToken     () const { return token_; }
  int                getPrecedence() const { return precedence_; }

  bool doesAssociateLtoR() const { return associateLtoR_; }
  bool isPunctuation    () const { return punctuation_; }
  bool isUnary          () const { return unary_; }
  bool isBinary         () const { return (! unary_ && ! punctuation_); }

 private:
  ExprOperator(ExprOperatorType type, const std::string &token, uint precedence,
               bool associateLtoR, bool punctuation, bool unary);

 private:
  static ExprOperator operators_[];
  static int          numOperators_;

  ExprOperatorType type_;
  std::string      token_;
  uint             precedence_;
  bool             associateLtoR_;
  bool             punctuation_;
  bool             unary_;
};

}

#endif
