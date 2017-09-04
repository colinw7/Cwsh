#include <CwshI.h>

CwshSet::
CwshSet(Cwsh *cwsh) :
 cwsh_(cwsh)
{
}

void
CwshSet::
parseSet(const std::string &str, std::string &name, int *index,
         CwshVariableType *type, std::vector<std::string> &values)
{
  uint i = 0;

  parseVariable(str, &i, name, index);

  uint len = str.size();

  if (i >= len || str[i] != '=')
    return;

  i++;

  setValues(str, &i, type, values);
}

void
CwshSet::
processSet(const std::string &name, int index, CwshVariableType type,
           std::vector<std::string> &values)
{
  if (index != -1) {
    int num_values = values.size();

    if (num_values != 1)
      CWSH_THROW("Syntax Error.");

    if (type == CwshVariableType::WORDLIST || num_values != 1)
      CWSH_THROW("Expression Syntax.");

    CwshVariable *variable = cwsh_->lookupVariable(name);

    if (! variable)
      CWSH_THROW("Undefined variable");

    if (index >= variable->getNumValues())
      CWSH_THROW("Subscript out of range.");

    variable->setValue(index - 1, values[0]);
  }
  else
    cwsh_->defineVariable(name, values);
}

void
CwshSet::
parseAssign(const std::string &str, std::string &name, int *index,
            CwshSetAssignType *assign_type, std::string &expr_str)
{
  uint i = 0;

  parseVariable(str, &i, name, index);

  *assign_type = parseAssignType(str, &i);

  if (*assign_type != CwshSetAssignType::INCREMENT &&
      *assign_type != CwshSetAssignType::DECREMENT) {
    CwshExprParse parse(cwsh_);

    expr_str = parse.parse(str, &i);
  }
}

void
CwshSet::
processAssign(const std::string &name, int index,
              CwshSetAssignType assign_type, const std::string &expr_str)
{
  int integer = 0;

  if (assign_type != CwshSetAssignType::INCREMENT &&
      assign_type != CwshSetAssignType::DECREMENT) {
    CwshExprEvaluate expr(cwsh_, expr_str);

    integer = expr.process();
  }

  CwshVariable *variable = cwsh_->lookupVariable(name);

  int integer1 = 0;

  if (index != -1) {
    if (! variable)
      CWSH_THROW("Undefined variable.");

    if (index >= variable->getNumValues())
      CWSH_THROW("Subscript out of range.");

    std::string str = variable->getValue(index - 1);

    if (! CStrUtil::isInteger(str))
      CWSH_THROW("Expression Syntax.");

    integer1 = CStrUtil::toInteger(str);
  }
  else {
    if (assign_type != CwshSetAssignType::EQUALS && ! variable)
      CWSH_THROW("Undefined variable.");

    if (variable && variable->getNumValues() > 0) {
      std::string str = variable->getValue(0);

      if (! CStrUtil::isInteger(str))
        CWSH_THROW("Expression Syntax.");

      integer1 = CStrUtil::toInteger(str);
    }
  }

  switch (assign_type) {
    case CwshSetAssignType::EQUALS:
      break;
    case CwshSetAssignType::PLUS_EQUALS:
      integer = integer1 + integer;
      break;
    case CwshSetAssignType::MINUS_EQUALS:
      integer = integer1 - integer;
      break;
    case CwshSetAssignType::TIMES_EQUALS:
      integer = integer1 * integer;
      break;
    case CwshSetAssignType::DIVIDE_EQUALS:
      if (integer == 0)
        CWSH_THROW("Division by 0.");

      integer = integer1 / integer;
      break;
    case CwshSetAssignType::MODULUS_EQUALS:
      if (integer == 0)
        CWSH_THROW("Mod by 0.");

      integer = integer1 % integer;
      break;
    case CwshSetAssignType::AND_EQUALS:
      integer = integer1 & integer;
      break;
    case CwshSetAssignType::OR_EQUALS:
      integer = integer1 | integer;
      break;
    case CwshSetAssignType::XOR_EQUALS:
      integer = integer1 ^ integer;
      break;
    case CwshSetAssignType::INCREMENT:
      integer = ++integer1;
      break;
    case CwshSetAssignType::DECREMENT:
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
CwshSet::
parseVariable(const std::string &str, uint *i, std::string &name, int *index)
{
  CStrUtil::skipSpace(str, i);

  uint len = str.size();

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

  *index = CStrUtil::toInteger(istr);

  CStrUtil::skipSpace(str, i);

  if (*i >= len || str[*i] != ']')
    CWSH_THROW("Subscript error.");

  (*i)++;
}

CwshSetAssignType
CwshSet::
parseAssignType(const std::string &str, uint *i)
{
  CwshSetAssignType type = CwshSetAssignType::NONE;

  CStrUtil::skipSpace(str, i);

  switch (str[*i]) {
    case '=':
      (*i)++;

      type = CwshSetAssignType::EQUALS;

      break;
    case '+':
      (*i)++;

      switch (str[*i]) {
        case '=':
          (*i)++;

          type = CwshSetAssignType::PLUS_EQUALS;

          break;
        case '+':
          (*i)++;

          type = CwshSetAssignType::INCREMENT;

          break;
      }

      break;
    case '-':
      (*i)++;

      switch (str[*i]) {
        case '=':
          (*i)++;

          type = CwshSetAssignType::MINUS_EQUALS;

          break;
        case '-':
          (*i)++;

          type = CwshSetAssignType::DECREMENT;

          break;
      }

      break;
    case '*':
      (*i)++;

      switch (str[*i]) {
        case '=':
          (*i)++;

          type = CwshSetAssignType::TIMES_EQUALS;

          break;
      }

      break;
    case '/':
      (*i)++;

      switch (str[*i]) {
        case '=':
          (*i)++;

          type = CwshSetAssignType::DIVIDE_EQUALS;

          break;
      }

      break;
    case '%':
      (*i)++;

      switch (str[*i]) {
        case '=':
          (*i)++;

          type = CwshSetAssignType::MODULUS_EQUALS;

          break;
      }

      break;
    case '&':
      (*i)++;

      switch (str[*i]) {
        case '=':
          (*i)++;

          type = CwshSetAssignType::AND_EQUALS;

          break;
      }

      break;
    case '|':
      (*i)++;

      switch (str[*i]) {
        case '=':
          (*i)++;

          type = CwshSetAssignType::OR_EQUALS;

          break;
      }

      break;
    case '^':
      (*i)++;

      switch (str[*i]) {
        case '=':
          (*i)++;

          type = CwshSetAssignType::XOR_EQUALS;

          break;
      }

      break;
  }

  if (type == CwshSetAssignType::NONE)
    CWSH_THROW("Syntax error.");

  CStrUtil::skipSpace(str, i);

  return type;
}

void
CwshSet::
setValues(const std::string &str, uint *i, CwshVariableType *type,
          std::vector<std::string> &values)
{
  CStrUtil::skipSpace(str, i);

  uint len = str.size();

  if (*i < len && str[*i] == '(') {
    *type = CwshVariableType::WORDLIST;

    (*i)++;

    int j = *i;

    CwshString::skipWordsToChar(str, i, ')');

    std::string str1 = str.substr(j, *i - j);

    (*i)++;

    CwshString::addWords(str1, values);
  }
  else {
    *type = CwshVariableType::WORD;

    int j = *i;

    CwshString::skipWord(str, i);

    std::string str1 = str.substr(j, *i - j);

    values.push_back(str1);
  }

  CStrUtil::skipSpace(str, i);
}
