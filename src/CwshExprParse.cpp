#include <CwshI.h>

namespace Cwsh {

std::string ExprParse::unaryOperatorChars_  = "+-!~";
std::string ExprParse::binaryOperatorChars_ = "+-*/%<>=!&|^#";
std::string ExprParse::fileOperatorChars_   = "deforwxz";

ExprParse::
ExprParse(App *cwsh) :
 cwsh_(cwsh)
{
}

std::string
ExprParse::
parse(const std::string &str, uint *pos)
{
  CStrUtil::skipSpace(str, pos);

  std::string expr;

  uint len = uint(str.size());

  if (*pos < len && str[*pos] == '(') {
    (*pos)++;

    CStrUtil::skipSpace(str, pos);

    uint j = *pos;

    String::skipWordsToChar(str, pos, ')');

    expr = str.substr(j, *pos - j);

    (*pos)++;
  }
  else
    expr = read(str, pos);

  CStrUtil::skipSpace(str, pos);

  expr = CStrUtil::stripSpaces(expr);

  return expr;
}

std::string
ExprParse::
read(const std::string &str, uint *pos)
{
  auto stack = std::make_unique<ExprStackStack>();

  uint j = *pos;

  subStack(stack.get(), str, pos);

  std::string expr = str.substr(j, *pos - j);

  return expr;
}

void
ExprParse::
stack(ExprStackStack *stack, const std::string &expr)
{
  uint pos = 0;

  subStack(stack, expr, &pos);
}

void
ExprParse::
subStack(ExprStackStack *stack, const std::string &str, uint *pos)
{
  bool isExpr = false;

  CStrUtil::skipSpace(str, pos);

  uint len = uint(str.size());

  while (*pos < len) {
    /* <expression> := <file_operator> <filename> */

    if      (! isExpr && str[*pos] == '-' &&
             fileOperatorChars_.find(str[*pos + 1]) != std::string::npos) {
      auto *opr = readFileOperator(str, pos);

      stack->push(opr);

      CStrUtil::skipSpace(str, pos);

      std::string value = readString(str, pos);

      auto word = Word(value);

      WordArray words;

      Pattern pattern(cwsh_);

      if (pattern.expandWordToFiles(word, words)) {
        uint numWords = uint(words.size());

        for (uint i = 0; i < numWords; i++)
          stack->push(words[i].getWord());
      }
      else
        stack->push(value);

      isExpr = true;
    }

    /* <expression> := <unary_operator> <expression> */

    else if (! isExpr && unaryOperatorChars_.find(str[*pos]) != std::string::npos) {
      auto *opr = readUnaryOperator(str, pos);

      stack->push(opr);

      subStack(stack, str, pos);

      isExpr = true;
    }

    /* <expression> := <expression> <binary_operator> <expression> */

    else if (isExpr && binaryOperatorChars_.find(str[*pos]) != std::string::npos) {
      auto *opr = readBinaryOperator(str, pos);

      stack->push(opr);

      subStack(stack, str, pos);

      isExpr = true;
    }

    /* <expression> := '(' <expression> ')' */

    else if (! isExpr && str[*pos] == '(') {
      auto *opr = ExprOperator::lookup(ExprOperatorType::OPEN_BRACKET);

      stack->push(opr);

      (*pos)++;

      CStrUtil::skipSpace(str, pos);

      uint pos1 = *pos;

      if (! skipToCloseBracket(str, pos) || str[*pos] != ')')
        CWSH_THROW("Missing Close Round Bracket.");

      std::string str1 = str.substr(pos1, *pos - pos1);

      (*pos)++;

      pos1 = 0;

      subStack(stack, str1, &pos1);

      CStrUtil::skipSpace(str1, &pos1);

      if (pos1 < str1.size())
        CWSH_THROW("Invalid Expression '" + str1 + "'");

      opr = ExprOperator::lookup(ExprOperatorType::CLOSE_BRACKET);

      stack->push(opr);

      isExpr = true;
    }

    /* <expression> := <integer> */

    else if (! isExpr && isdigit(str[*pos])) {
      std::string value = readInteger(str, pos);

      stack->push(value);

      isExpr = true;
    }

    /* <expression> := <string_value> */

    else if (! isExpr) {
      std::string value = readString(str, pos);

      stack->push(value);

      isExpr = true;
    }
    else
      break;

    CStrUtil::skipSpace(str, pos);
  }

  if (! isExpr)
    CWSH_THROW("Null Expression.");
}

ExprOperator *
ExprParse::
readFileOperator(const std::string &str, uint *pos)
{
  ExprOperator *opr = nullptr;

  if (str[*pos] != '-')
    CWSH_THROW("Invalid Operator.");

  (*pos)++;

  switch (str[*pos]) {
    case 'd':
      opr = ExprOperator::lookup(ExprOperatorType::IS_DIRECTORY);
      break;
    case 'e':
      opr = ExprOperator::lookup(ExprOperatorType::IS_FILE);
      break;
    case 'f':
      opr = ExprOperator::lookup(ExprOperatorType::IS_PLAIN);
      break;
    case 'o':
      opr = ExprOperator::lookup(ExprOperatorType::IS_OWNER);
      break;
    case 'r':
      opr = ExprOperator::lookup(ExprOperatorType::IS_READABLE);
      break;
    case 'w':
      opr = ExprOperator::lookup(ExprOperatorType::IS_WRITABLE);
      break;
    case 'x':
      opr = ExprOperator::lookup(ExprOperatorType::IS_EXECUTABLE);
      break;
    case 'z':
      opr = ExprOperator::lookup(ExprOperatorType::IS_ZERO);
      break;
    default:
      CWSH_THROW("Invalid Operator.");
      break;
  }

  (*pos)++;

  return opr;
}

ExprOperator *
ExprParse::
readUnaryOperator(const std::string &str, uint *pos)
{
  ExprOperator *opr = nullptr;

  switch (str[*pos]) {
    case '+':
      opr = ExprOperator::lookup(ExprOperatorType::UNARY_PLUS);

      (*pos)++;

      break;
    case '-':
      opr = ExprOperator::lookup(ExprOperatorType::UNARY_MINUS);

      (*pos)++;

      break;
    case '!':
      opr = ExprOperator::lookup(ExprOperatorType::LOGICAL_NOT);

      (*pos)++;

      break;
    case '~':
      opr = ExprOperator::lookup(ExprOperatorType::BIT_NOT);

      (*pos)++;

      break;
  }

  return opr;
}

ExprOperator *
ExprParse::
readBinaryOperator(const std::string &str, uint *pos)
{
  ExprOperator *opr = nullptr;

  switch (str[*pos]) {
    case '+':
      opr = ExprOperator::lookup(ExprOperatorType::PLUS);

      (*pos)++;

      break;
    case '-':
      opr = ExprOperator::lookup(ExprOperatorType::MINUS);

      (*pos)++;

      break;
    case '*':
      opr = ExprOperator::lookup(ExprOperatorType::TIMES);

      (*pos)++;

      break;
    case '/':
      opr = ExprOperator::lookup(ExprOperatorType::DIVIDE);

      (*pos)++;

      break;
    case '^':
      opr = ExprOperator::lookup(ExprOperatorType::BIT_XOR);

      (*pos)++;

      break;
    case '<':
      if      (str[*pos + 1] == '=') {
        opr = ExprOperator::lookup(ExprOperatorType::LESS_OR_EQUAL);

        (*pos)++;
      }
      else if (str[*pos + 1] == '<') {
        opr = ExprOperator::lookup(ExprOperatorType::BIT_LSHIFT);

        (*pos)++;
      }
      else
        opr = ExprOperator::lookup(ExprOperatorType::LESS);

      (*pos)++;

      break;
    case '>':
      if      (str[*pos + 1] == '=') {
        opr = ExprOperator::lookup(ExprOperatorType::GREATER_OR_EQUAL);

        (*pos)++;
      }
      else if (str[*pos + 1] == '>') {
        opr = ExprOperator::lookup(ExprOperatorType::BIT_RSHIFT);

        (*pos)++;
      }
      else
        opr = ExprOperator::lookup(ExprOperatorType::GREATER);

      (*pos)++;

      break;
    case '=':
      if      (str[*pos + 1] == '=') {
        opr = ExprOperator::lookup(ExprOperatorType::EQUAL);

        (*pos)++;
        (*pos)++;
      }
      else if (str[*pos + 1] == '~') {
        opr = ExprOperator::lookup(ExprOperatorType::MATCH_EQUAL);

        (*pos)++;
        (*pos)++;
      }

      break;
    case '!':
      if      (str[*pos + 1] == '=') {
        opr = ExprOperator::lookup(ExprOperatorType::NOT_EQUAL);

        (*pos)++;
        (*pos)++;
      }
      else if (str[*pos + 1] == '~') {
        opr = ExprOperator::lookup(ExprOperatorType::NO_MATCH_EQUAL);

        (*pos)++;
        (*pos)++;
      }
      else
        CWSH_THROW("Invalid Operator.");

      break;
    case '&':
      if      (str[*pos + 1] == '&') {
        opr = ExprOperator::lookup(ExprOperatorType::LOGICAL_AND);

        (*pos)++;
      }
      else
        opr = ExprOperator::lookup(ExprOperatorType::BIT_AND);

      (*pos)++;

      break;
    case '|':
      if      (str[*pos + 1] == '|') {
        opr = ExprOperator::lookup(ExprOperatorType::LOGICAL_OR);

        (*pos)++;
      }
      else
        opr = ExprOperator::lookup(ExprOperatorType::BIT_OR);

      (*pos)++;

      break;
    case '%':
      opr = ExprOperator::lookup(ExprOperatorType::MODULUS);

      (*pos)++;

      break;
    default:
      break;
  }

  return opr;
}

std::string
ExprParse::
readInteger(const std::string &str, uint *pos)
{
  uint pos1 = *pos;

  if (! CStrUtil::skipInteger(str, pos))
    CWSH_THROW("Invalid Integer Value.");

  std::string value = str.substr(pos1, *pos - pos1);

  return value;
}

std::string
ExprParse::
readString(const std::string &str, uint *pos)
{
  std::string value = "";

  uint len = uint(str.size());

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
ExprParse::
skipToCloseBracket(const std::string &str, uint *pos)
{
  uint brackets = 0;

  uint len = uint(str.size());

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

}
