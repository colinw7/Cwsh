#include <CwshI.h>

namespace Cwsh {

Set::
Set(App *cwsh) :
 cwsh_(cwsh)
{
}

void
Set::
parseSet(const std::string &str, std::string &name, int *index,
         VariableType *type, std::vector<std::string> &values)
{
  uint i = 0;

  parseVariable(str, &i, name, index);

  uint len = uint(str.size());

  if (i >= len || str[i] != '=')
    return;

  i++;

  setValues(str, &i, type, values);
}

void
Set::
processSet(const std::string &name, int index, VariableType type,
           std::vector<std::string> &values)
{
  if (index != -1) {
    auto num_values = values.size();

    if (num_values != 1)
      CWSH_THROW("Syntax Error.");

    if (type == VariableType::WORDLIST || num_values != 1)
      CWSH_THROW("Expression Syntax.");

    auto *variable = cwsh_->lookupVariable(name);

    if (! variable)
      CWSH_THROW("Undefined variable");

    if (index >= int(variable->getNumValues()))
      CWSH_THROW("Subscript out of range.");

    variable->setValue(index - 1, values[0]);
  }
  else
    cwsh_->defineVariable(name, values);
}

void
Set::
parseAssign(const std::string &str, std::string &name, int *index,
            SetAssignType *assignType, std::string &expr_str)
{
  uint i = 0;

  parseVariable(str, &i, name, index);

  *assignType = parseAssignType(str, &i);

  if (*assignType != SetAssignType::INCREMENT &&
      *assignType != SetAssignType::DECREMENT) {
    ExprParse parse(cwsh_);

    expr_str = parse.parse(str, &i);
  }
}

void
Set::
processAssign(const std::string &name, int index,
              SetAssignType assignType, const std::string &expr_str)
{
  int integer = 0;

  if (assignType != SetAssignType::INCREMENT &&
      assignType != SetAssignType::DECREMENT) {
    ExprEvaluate expr(cwsh_, expr_str);

    integer = expr.process();
  }

  auto *variable = cwsh_->lookupVariable(name);

  int integer1 = 0;

  if (index != -1) {
    if (! variable)
      CWSH_THROW("Undefined variable.");

    if (index >= int(variable->getNumValues()))
      CWSH_THROW("Subscript out of range.");

    std::string str = variable->getValue(index - 1);

    if (! CStrUtil::isInteger(str))
      CWSH_THROW("Expression Syntax.");

    integer1 = int(CStrUtil::toInteger(str));
  }
  else {
    if (assignType != SetAssignType::EQUALS && ! variable)
      CWSH_THROW("Undefined variable.");

    if (variable && variable->getNumValues() > 0) {
      std::string str = variable->getValue(0);

      if (! CStrUtil::isInteger(str))
        CWSH_THROW("Expression Syntax.");

      integer1 = int(CStrUtil::toInteger(str));
    }
  }

  switch (assignType) {
    case SetAssignType::EQUALS:
      break;
    case SetAssignType::PLUS_EQUALS:
      integer = integer1 + integer;
      break;
    case SetAssignType::MINUS_EQUALS:
      integer = integer1 - integer;
      break;
    case SetAssignType::TIMES_EQUALS:
      integer = integer1 * integer;
      break;
    case SetAssignType::DIVIDE_EQUALS:
      if (integer == 0)
        CWSH_THROW("Division by 0.");

      integer = integer1 / integer;
      break;
    case SetAssignType::MODULUS_EQUALS:
      if (integer == 0)
        CWSH_THROW("Mod by 0.");

      integer = integer1 % integer;
      break;
    case SetAssignType::AND_EQUALS:
      integer = integer1 & integer;
      break;
    case SetAssignType::OR_EQUALS:
      integer = integer1 | integer;
      break;
    case SetAssignType::XOR_EQUALS:
      integer = integer1 ^ integer;
      break;
    case SetAssignType::INCREMENT:
      integer = ++integer1;
      break;
    case SetAssignType::DECREMENT:
      integer = --integer1;
      break;
    default:
      break;
  }

  if (index != -1)
    variable->setValue(index - 1, CStrUtil::toString(integer));
  else
    cwsh_->defineVariable(name, integer);
}

void
Set::
parseVariable(const std::string &str, uint *i, std::string &name, int *index)
{
  CStrUtil::skipSpace(str, i);

  uint len = uint(str.size());

  if (*i >= len || (str[*i] != '_' && ! isalpha(str[*i])))
    CWSH_THROW("Variable name must begin with a letter.");

  int j = *i;

  (*i)++;

  while (*i < len && (str[*i] == '_' || isalnum(str[*i])))
    (*i)++;

  name = str.substr(j, *i - j);

  CStrUtil::skipSpace(str, i);

  if (*i < len || str[*i] != '[') {
    *index = -1;
    return;
  }

  (*i)++;

  if (*i >= len || ! isdigit(str[*i]))
    CWSH_THROW("Subscript error.");

  j = *i;

  while (str[*i] != '\0' && isdigit(str[*i]))
    (*i)++;

  std::string istr = str.substr(j, *i - j);

  if (! CStrUtil::isInteger(istr))
    throw "set: Subscript error.";

  *index = int(CStrUtil::toInteger(istr));

  CStrUtil::skipSpace(str, i);

  if (*i >= len || str[*i] != ']')
    CWSH_THROW("Subscript error.");

  (*i)++;
}

SetAssignType
Set::
parseAssignType(const std::string &str, uint *i)
{
  auto type = SetAssignType::NONE;

  CStrUtil::skipSpace(str, i);

  switch (str[*i]) {
    case '=':
      (*i)++;

      type = SetAssignType::EQUALS;

      break;
    case '+':
      (*i)++;

      switch (str[*i]) {
        case '=':
          (*i)++;

          type = SetAssignType::PLUS_EQUALS;

          break;
        case '+':
          (*i)++;

          type = SetAssignType::INCREMENT;

          break;
      }

      break;
    case '-':
      (*i)++;

      switch (str[*i]) {
        case '=':
          (*i)++;

          type = SetAssignType::MINUS_EQUALS;

          break;
        case '-':
          (*i)++;

          type = SetAssignType::DECREMENT;

          break;
      }

      break;
    case '*':
      (*i)++;

      switch (str[*i]) {
        case '=':
          (*i)++;

          type = SetAssignType::TIMES_EQUALS;

          break;
      }

      break;
    case '/':
      (*i)++;

      switch (str[*i]) {
        case '=':
          (*i)++;

          type = SetAssignType::DIVIDE_EQUALS;

          break;
      }

      break;
    case '%':
      (*i)++;

      switch (str[*i]) {
        case '=':
          (*i)++;

          type = SetAssignType::MODULUS_EQUALS;

          break;
      }

      break;
    case '&':
      (*i)++;

      switch (str[*i]) {
        case '=':
          (*i)++;

          type = SetAssignType::AND_EQUALS;

          break;
      }

      break;
    case '|':
      (*i)++;

      switch (str[*i]) {
        case '=':
          (*i)++;

          type = SetAssignType::OR_EQUALS;

          break;
      }

      break;
    case '^':
      (*i)++;

      switch (str[*i]) {
        case '=':
          (*i)++;

          type = SetAssignType::XOR_EQUALS;

          break;
      }

      break;
  }

  if (type == SetAssignType::NONE)
    CWSH_THROW("Syntax error.");

  CStrUtil::skipSpace(str, i);

  return type;
}

void
Set::
setValues(const std::string &str, uint *i, VariableType *type,
          std::vector<std::string> &values)
{
  CStrUtil::skipSpace(str, i);

  uint len = uint(str.size());

  if (*i < len && str[*i] == '(') {
    *type = VariableType::WORDLIST;

    (*i)++;

    int j = *i;

    String::skipWordsToChar(str, i, ')');

    std::string str1 = str.substr(j, *i - j);

    (*i)++;

    String::addWords(str1, values);
  }
  else {
    *type = VariableType::WORD;

    int j = *i;

    String::skipWord(str, i);

    std::string str1 = str.substr(j, *i - j);

    values.push_back(str1);
  }

  CStrUtil::skipSpace(str, i);
}

}
