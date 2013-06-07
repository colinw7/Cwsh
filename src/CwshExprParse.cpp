#include "CwshI.h"

string CwshExprParse::unary_operator_chars_  = "+-!~";
string CwshExprParse::binary_operator_chars_ = "+-*/%<>=!&|^#";
string CwshExprParse::file_operator_chars_   = "deforwxz";

CwshExprParse::
CwshExprParse(Cwsh *cwsh) :
 cwsh_(cwsh)
{
}

string
CwshExprParse::
parse(const string &str, uint *pos)
{
  CStrUtil::skipSpace(str, pos);

  string expr;

  uint len = str.size();

  if (*pos < len && str[*pos] == '(') {
    (*pos)++;

    CStrUtil::skipSpace(str, pos);

    uint j = *pos;

    CwshString::skipWordsToChar(str, pos, ')');

    expr = str.substr(j, *pos - j);

    (*pos)++;
  }
  else
    expr = read(str, pos);

  CStrUtil::skipSpace(str, pos);

  expr = CStrUtil::stripSpaces(expr);

  return expr;
}

string
CwshExprParse::
read(const string &str, uint *pos)
{
  CAutoPtr<CwshExprStackStack> stack;

  stack = new CwshExprStackStack();

  uint j = *pos;

  subStack(stack, str, pos);

  string expr = str.substr(j, *pos - j);

  return expr;
}

void
CwshExprParse::
stack(CwshExprStackStack *stack, const string &expr)
{
  uint pos = 0;

  subStack(stack, expr, &pos);
}

void
CwshExprParse::
subStack(CwshExprStackStack *stack, const string &str, uint *pos)
{
  bool is_expr = false;

  CStrUtil::skipSpace(str, pos);

  uint len = str.size();

  while (*pos < len) {
    /* <expression> := <file_operator> <filename> */

    if      (! is_expr && str[*pos] == '-' &&
             file_operator_chars_.find(str[*pos + 1]) != string::npos) {
      CwshExprOperator *opr = readFileOperator(str, pos);

      stack->push(opr);

      CStrUtil::skipSpace(str, pos);

      string value = readString(str, pos);

      CwshWord word = value;

      CwshWordArray words;

      CwshPattern pattern(cwsh_);

      if (pattern.expandWordToFiles(word, words)) {
        uint num_words = words.size();

        for (uint i = 0; i < num_words; i++)
          stack->push(words[i].getWord());
      }
      else
        stack->push(value);

      is_expr = true;
    }

    /* <expression> := <unary_operator> <expression> */

    else if (! is_expr &&
             unary_operator_chars_.find(str[*pos]) != string::npos) {
      CwshExprOperator *opr = readUnaryOperator(str, pos);

      stack->push(opr);

      subStack(stack, str, pos);

      is_expr = true;
    }

    /* <expression> := <expression> <binary_operator> <expression> */

    else if (is_expr &&
             binary_operator_chars_.find(str[*pos]) != string::npos) {
      CwshExprOperator *opr = readBinaryOperator(str, pos);

      stack->push(opr);

      subStack(stack, str, pos);

      is_expr = true;
    }

    /* <expression> := '(' <expression> ')' */

    else if (! is_expr && str[*pos] == '(') {
      CwshExprOperator *opr =
        CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_OPEN_BRACKET);

      stack->push(opr);

      (*pos)++;

      CStrUtil::skipSpace(str, pos);

      uint pos1 = *pos;

      if (! skipToCloseBracket(str, pos) || str[*pos] != ')')
        CWSH_THROW("Missing Close Round Bracket.");

      string str1 = str.substr(pos1, *pos - pos1);

      (*pos)++;

      pos1 = 0;

      subStack(stack, str1, &pos1);

      CStrUtil::skipSpace(str1, &pos1);

      if (pos1 < str1.size())
        CWSH_THROW("Invalid Expression '" + str1 + "'");

      opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_CLOSE_BRACKET);

      stack->push(opr);

      is_expr = true;
    }

    /* <expression> := <integer> */

    else if (! is_expr && isdigit(str[*pos])) {
      string value = readInteger(str, pos);

      stack->push(value);

      is_expr = true;
    }

    /* <expression> := <string_value> */

    else if (! is_expr) {
      string value = readString(str, pos);

      stack->push(value);

      is_expr = true;
    }
    else
      break;

    CStrUtil::skipSpace(str, pos);
  }

  if (! is_expr)
    CWSH_THROW("Null Expression.");
}

CwshExprOperator *
CwshExprParse::
readFileOperator(const string &str, uint *pos)
{
  CwshExprOperator *opr = NULL;

  if (str[*pos] != '-')
    CWSH_THROW("Invalid Operator.");

  (*pos)++;

  switch (str[*pos]) {
    case 'd':
      opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_IS_DIRECTORY);
      break;
    case 'e':
      opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_IS_FILE);
      break;
    case 'f':
      opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_IS_PLAIN);
      break;
    case 'o':
      opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_IS_OWNER);
      break;
    case 'r':
      opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_IS_READABLE);
      break;
    case 'w':
      opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_IS_WRITABLE);
      break;
    case 'x':
      opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_IS_EXECUTABLE);
      break;
    case 'z':
      opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_IS_ZERO);
      break;
    default:
      CWSH_THROW("Invalid Operator.");
      break;
  }

  (*pos)++;

  return opr;
}

CwshExprOperator *
CwshExprParse::
readUnaryOperator(const string &str, uint *pos)
{
  CwshExprOperator *opr = NULL;

  switch (str[*pos]) {
    case '+':
      opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_UNARY_PLUS);

      (*pos)++;

      break;
    case '-':
      opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_UNARY_MINUS);

      (*pos)++;

      break;
    case '!':
      opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_LOGICAL_NOT);

      (*pos)++;

      break;
    case '~':
      opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_BIT_NOT);

      (*pos)++;

      break;
  }

  return opr;
}

CwshExprOperator *
CwshExprParse::
readBinaryOperator(const string &str, uint *pos)
{
  CwshExprOperator *opr = NULL;

  switch (str[*pos]) {
    case '+':
      opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_PLUS);

      (*pos)++;

      break;
    case '-':
      opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_MINUS);

      (*pos)++;

      break;
    case '*':
      opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_TIMES);

      (*pos)++;

      break;
    case '/':
      opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_DIVIDE);

      (*pos)++;

      break;
    case '^':
      opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_BIT_XOR);

      (*pos)++;

      break;
    case '<':
      if      (str[*pos + 1] == '=') {
        opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_LESS_OR_EQUAL);

        (*pos)++;
      }
      else if (str[*pos + 1] == '<') {
        opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_BIT_LSHIFT);

        (*pos)++;
      }
      else
        opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_LESS);

      (*pos)++;

      break;
    case '>':
      if      (str[*pos + 1] == '=') {
        opr = CwshExprOperator::lookup
               (CWSH_EXPR_OPERATOR_TYPE_GREATER_OR_EQUAL);

        (*pos)++;
      }
      else if (str[*pos + 1] == '>') {
        opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_BIT_RSHIFT);

        (*pos)++;
      }
      else
        opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_GREATER);

      (*pos)++;

      break;
    case '=':
      if      (str[*pos + 1] == '=') {
        opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_EQUAL);

        (*pos)++;
        (*pos)++;
      }
      else if (str[*pos + 1] == '~') {
        opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_MATCH_EQUAL);

        (*pos)++;
        (*pos)++;
      }

      break;
    case '!':
      if      (str[*pos + 1] == '=') {
        opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_NOT_EQUAL);

        (*pos)++;
        (*pos)++;
      }
      else if (str[*pos + 1] == '~') {
        opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_NO_MATCH_EQUAL);

        (*pos)++;
        (*pos)++;
      }
      else
        CWSH_THROW("Invalid Operator.");

      break;
    case '&':
      if      (str[*pos + 1] == '&') {
        opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_LOGICAL_AND);

        (*pos)++;
      }
      else
        opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_BIT_AND);

      (*pos)++;

      break;
    case '|':
      if      (str[*pos + 1] == '|') {
        opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_LOGICAL_OR);

        (*pos)++;
      }
      else
        opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_BIT_OR);

      (*pos)++;

      break;
    case '%':
      opr = CwshExprOperator::lookup(CWSH_EXPR_OPERATOR_TYPE_MODULUS);

      (*pos)++;

      break;
    default:
      break;
  }

  return opr;
}

string
CwshExprParse::
readInteger(const string &str, uint *pos)
{
  uint pos1 = *pos;

  if (! CStrUtil::skipInteger(str, pos))
    CWSH_THROW("Invalid Integer Value.");

  string value = str.substr(pos1, *pos - pos1);

  return value;
}

string
CwshExprParse::
readString(const string &str, uint *pos)
{
  string value = "";

  uint len = str.size();

  while (*pos < len) {
    if      (str[*pos] == '\"') {
      uint pos1 = *pos;

      if (! CStrUtil::skipDoubleQuotedString(str, pos))
        CWSH_THROW("Invalid String Value.");

      value += str.substr(pos1, *pos - pos1);
    }
    else if (str[*pos] == '\'') {
      uint pos1 = *pos;

      if (! CStrUtil::skipSingleQuotedString(str, pos))
        CWSH_THROW("Invalid String Value.");

      value += str.substr(pos1, *pos - pos1);
    }
    else if (isspace(str[*pos]))
      break;
    else
      value += str[(*pos)++];
  }

  return value;
}

bool
CwshExprParse::
skipToCloseBracket(const string &str, uint *pos)
{
  uint brackets = 0;

  uint len = str.size();

  while (*pos < len) {
    if      (str[*pos] == '(') {
      ++brackets;

      ++(*pos);
    }
    else if (str[*pos] == ')') {
      if (brackets == 0)
        return true;

      --brackets;

      ++(*pos);
    }
    else if (str[*pos] == '\"') {
      if (! CStrUtil::skipDoubleQuotedString(str, pos))
        CWSH_THROW("Invalid String Value.");
    }
    else if (str[*pos] == '\'') {
      if (! CStrUtil::skipSingleQuotedString(str, pos))
        CWSH_THROW("Invalid String Value.");
    }
    else
      ++(*pos);
  }

  return false;
}
