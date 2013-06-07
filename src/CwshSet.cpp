#include "CwshI.h"

CwshSet::
CwshSet(Cwsh *cwsh) :
 cwsh_(cwsh)
{
}

void
CwshSet::
parseSet(const string &str, string &name, int *index,
         CwshVariableType *type, vector<string> &values)
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
processSet(const string &name, int index, CwshVariableType type,
           vector<string> &values)
{
  if (index != -1) {
    int num_values = values.size();

    if (num_values != 1)
      CWSH_THROW("Syntax Error.");

    if (type == CWSH_VARIABLE_TYPE_WORDLIST || num_values != 1)
      CWSH_THROW("Expression Syntax.");

    CwshVariable *variable = cwsh_->lookupVariable(name);

    if (variable == NULL)
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
parseAssign(const string &str, string &name, int *index,
            CwshSetAssignType *assign_type, string &expr_str)
{
  uint i = 0;

  parseVariable(str, &i, name, index);

  *assign_type = parseAssignType(str, &i);

  if (*assign_type != CWSH_SET_ASSIGN_TYPE_INCREMENT &&
      *assign_type != CWSH_SET_ASSIGN_TYPE_DECREMENT) {
    CwshExprParse parse(cwsh_);

    expr_str = parse.parse(str, &i);
  }
}

void
CwshSet::
processAssign(const string &name, int index,
              CwshSetAssignType assign_type, const string &expr_str)
{
  int integer = 0;

  if (assign_type != CWSH_SET_ASSIGN_TYPE_INCREMENT &&
      assign_type != CWSH_SET_ASSIGN_TYPE_DECREMENT) {
    CwshExprEvaluate expr(cwsh_, expr_str);

    integer = expr.process();
  }

  CwshVariable *variable = cwsh_->lookupVariable(name);

  int integer1 = 0;

  if (index != -1) {
    if (variable == NULL)
      CWSH_THROW("Undefined variable.");

    if (index >= variable->getNumValues())
      CWSH_THROW("Subscript out of range.");

    string str = variable->getValue(index - 1);

    if (! CStrUtil::isInteger(str))
      CWSH_THROW("Expression Syntax.");

    integer1 = CStrUtil::toInteger(str);
  }
  else {
    if (assign_type != CWSH_SET_ASSIGN_TYPE_EQUALS && variable == NULL)
      CWSH_THROW("Undefined variable.");

    if (variable != NULL && variable->getNumValues() > 0) {
      string str = variable->getValue(0);

      if (! CStrUtil::isInteger(str))
        CWSH_THROW("Expression Syntax.");

      integer1 = CStrUtil::toInteger(str);
    }
  }

  switch (assign_type) {
    case CWSH_SET_ASSIGN_TYPE_EQUALS:
      break;
    case CWSH_SET_ASSIGN_TYPE_PLUS_EQUALS:
      integer = integer1 + integer;
      break;
    case CWSH_SET_ASSIGN_TYPE_MINUS_EQUALS:
      integer = integer1 - integer;
      break;
    case CWSH_SET_ASSIGN_TYPE_TIMES_EQUALS:
      integer = integer1 * integer;
      break;
    case CWSH_SET_ASSIGN_TYPE_DIVIDE_EQUALS:
      if (integer == 0)
        CWSH_THROW("Division by 0.");

      integer = integer1 / integer;
      break;
    case CWSH_SET_ASSIGN_TYPE_MODULUS_EQUALS:
      if (integer == 0)
        CWSH_THROW("Mod by 0.");

      integer = integer1 % integer;
      break;
    case CWSH_SET_ASSIGN_TYPE_AND_EQUALS:
      integer = integer1 & integer;
      break;
    case CWSH_SET_ASSIGN_TYPE_OR_EQUALS:
      integer = integer1 | integer;
      break;
    case CWSH_SET_ASSIGN_TYPE_XOR_EQUALS:
      integer = integer1 ^ integer;
      break;
    case CWSH_SET_ASSIGN_TYPE_INCREMENT:
      integer = ++integer1;
      break;
    case CWSH_SET_ASSIGN_TYPE_DECREMENT:
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
parseVariable(const string &str, uint *i, string &name, int *index)
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

  string istr = str.substr(j, *i - j);

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
parseAssignType(const string &str, uint *i)
{
  CwshSetAssignType type = CWSH_SET_ASSIGN_TYPE_NONE;

  CStrUtil::skipSpace(str, i);

  switch (str[*i]) {
    case '=':
      (*i)++;

      type = CWSH_SET_ASSIGN_TYPE_EQUALS;

      break;
    case '+':
      (*i)++;

      switch (str[*i]) {
        case '=':
          (*i)++;

          type = CWSH_SET_ASSIGN_TYPE_PLUS_EQUALS;

          break;
        case '+':
          (*i)++;

          type = CWSH_SET_ASSIGN_TYPE_INCREMENT;

          break;
      }

      break;
    case '-':
      (*i)++;

      switch (str[*i]) {
        case '=':
          (*i)++;

          type = CWSH_SET_ASSIGN_TYPE_MINUS_EQUALS;

          break;
        case '-':
          (*i)++;

          type = CWSH_SET_ASSIGN_TYPE_DECREMENT;

          break;
      }

      break;
    case '*':
      (*i)++;

      switch (str[*i]) {
        case '=':
          (*i)++;

          type = CWSH_SET_ASSIGN_TYPE_TIMES_EQUALS;

          break;
      }

      break;
    case '/':
      (*i)++;

      switch (str[*i]) {
        case '=':
          (*i)++;

          type = CWSH_SET_ASSIGN_TYPE_DIVIDE_EQUALS;

          break;
      }

      break;
    case '%':
      (*i)++;

      switch (str[*i]) {
        case '=':
          (*i)++;

          type = CWSH_SET_ASSIGN_TYPE_MODULUS_EQUALS;

          break;
      }

      break;
    case '&':
      (*i)++;

      switch (str[*i]) {
        case '=':
          (*i)++;

          type = CWSH_SET_ASSIGN_TYPE_AND_EQUALS;

          break;
      }

      break;
    case '|':
      (*i)++;

      switch (str[*i]) {
        case '=':
          (*i)++;

          type = CWSH_SET_ASSIGN_TYPE_OR_EQUALS;

          break;
      }

      break;
    case '^':
      (*i)++;

      switch (str[*i]) {
        case '=':
          (*i)++;

          type = CWSH_SET_ASSIGN_TYPE_XOR_EQUALS;

          break;
      }

      break;
  }

  if (type == -1)
    CWSH_THROW("Syntax error.");

  CStrUtil::skipSpace(str, i);

  return type;
}

void
CwshSet::
setValues(const string &str, uint *i, CwshVariableType *type,
          vector<string> &values)
{
  CStrUtil::skipSpace(str, i);

  uint len = str.size();

  if (*i < len && str[*i] == '(') {
    *type = CWSH_VARIABLE_TYPE_WORDLIST;

    (*i)++;

    int j = *i;

    CwshString::skipWordsToChar(str, i, ')');

    string str1 = str.substr(j, *i - j);

    (*i)++;

    CwshString::addWords(str1, values);
  }
  else {
    *type = CWSH_VARIABLE_TYPE_WORD;

    int j = *i;

    CwshString::skipWord(str, i);

    string str1 = str.substr(j, *i - j);

    values.push_back(str1);
  }

  CStrUtil::skipSpace(str, i);
}
