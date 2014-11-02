#include <CwshI.h>

CwshExprProcess::
CwshExprProcess()
{
}

string
CwshExprProcess::
process(CwshExprOperator *opr, const string &value)
{
  int integer;

  CwshExprProcessValueType type = getValueType(value, &integer);

  switch (opr->getType()) {
    case CWSH_EXPR_OPERATOR_TYPE_UNARY_PLUS:
    case CWSH_EXPR_OPERATOR_TYPE_UNARY_MINUS:
    case CWSH_EXPR_OPERATOR_TYPE_LOGICAL_NOT:
    case CWSH_EXPR_OPERATOR_TYPE_BIT_NOT:
      if (type != CWSH_EXPR_PROCESS_VALUE_TYPE_INTEGER)
        CWSH_THROW("Invalid Type for Operator.");

      break;
    case CWSH_EXPR_OPERATOR_TYPE_IS_DIRECTORY:
    case CWSH_EXPR_OPERATOR_TYPE_IS_FILE:
    case CWSH_EXPR_OPERATOR_TYPE_IS_PLAIN:
    case CWSH_EXPR_OPERATOR_TYPE_IS_OWNER:
    case CWSH_EXPR_OPERATOR_TYPE_IS_READABLE:
    case CWSH_EXPR_OPERATOR_TYPE_IS_WRITABLE:
    case CWSH_EXPR_OPERATOR_TYPE_IS_EXECUTABLE:
    case CWSH_EXPR_OPERATOR_TYPE_IS_ZERO:
      if (type != CWSH_EXPR_PROCESS_VALUE_TYPE_STRING)
        CWSH_THROW("Invalid Type for Operator.");

      break;
    default:
      break;
  }

  switch (opr->getType()) {
    case CWSH_EXPR_OPERATOR_TYPE_UNARY_PLUS:
      break;
    case CWSH_EXPR_OPERATOR_TYPE_UNARY_MINUS:
      integer = -integer;

      break;
    case CWSH_EXPR_OPERATOR_TYPE_LOGICAL_NOT:
      if (! integer)
        integer = true;
      else
        integer = false;

      break;
    case CWSH_EXPR_OPERATOR_TYPE_BIT_NOT:
      integer = ~integer;

      break;
    case CWSH_EXPR_OPERATOR_TYPE_IS_DIRECTORY:
      integer = (CFile::exists(value) && CFile::isDirectory(value));

      break;
    case CWSH_EXPR_OPERATOR_TYPE_IS_FILE:
      integer = CFile::exists(value);

      break;
    case CWSH_EXPR_OPERATOR_TYPE_IS_PLAIN:
      integer = (CFile::exists(value) && CFile::isRegular(value));

      break;
    case CWSH_EXPR_OPERATOR_TYPE_IS_OWNER:
      integer = (CFile::exists(value) && CFile::isOwner(value));

      break;
    case CWSH_EXPR_OPERATOR_TYPE_IS_READABLE:
      integer = (CFile::exists(value) && CFile::isReadable(value));

      break;
    case CWSH_EXPR_OPERATOR_TYPE_IS_WRITABLE:
      integer = (CFile::exists(value) && CFile::isWritable(value));

      break;
    case CWSH_EXPR_OPERATOR_TYPE_IS_EXECUTABLE:
      integer = (CFile::exists(value) && CFile::isExecutable(value));

      break;
    case CWSH_EXPR_OPERATOR_TYPE_IS_ZERO:
      integer = (CFile::exists(value) && CFile::getSize(value) == 0);

      break;
    default:
      break;
  }

  string value1 = CStrUtil::toString(integer);

  return value1;
}

string
CwshExprProcess::
process(const string &value1, CwshExprOperator *opr, const string &value2)
{
  int integer1;
  int integer2;

  CwshExprProcessValueType type1 = getValueType(value1, &integer1);
  CwshExprProcessValueType type2 = getValueType(value2, &integer2);

  switch (opr->getType()) {
    case CWSH_EXPR_OPERATOR_TYPE_PLUS:
    case CWSH_EXPR_OPERATOR_TYPE_MINUS:
    case CWSH_EXPR_OPERATOR_TYPE_TIMES:
    case CWSH_EXPR_OPERATOR_TYPE_DIVIDE:
    case CWSH_EXPR_OPERATOR_TYPE_MODULUS:
    case CWSH_EXPR_OPERATOR_TYPE_LOGICAL_AND:
    case CWSH_EXPR_OPERATOR_TYPE_LOGICAL_OR:
    case CWSH_EXPR_OPERATOR_TYPE_BIT_AND:
    case CWSH_EXPR_OPERATOR_TYPE_BIT_OR:
    case CWSH_EXPR_OPERATOR_TYPE_BIT_XOR:
    case CWSH_EXPR_OPERATOR_TYPE_BIT_LSHIFT:
    case CWSH_EXPR_OPERATOR_TYPE_BIT_RSHIFT:
      if (type1 != CWSH_EXPR_PROCESS_VALUE_TYPE_INTEGER ||
          type2 != CWSH_EXPR_PROCESS_VALUE_TYPE_INTEGER)
        CWSH_THROW("Invalid Type for Operator.");

      break;

    case CWSH_EXPR_OPERATOR_TYPE_MATCH_EQUAL:
    case CWSH_EXPR_OPERATOR_TYPE_NO_MATCH_EQUAL:
      if (type1 != CWSH_EXPR_PROCESS_VALUE_TYPE_STRING ||
          type2 != CWSH_EXPR_PROCESS_VALUE_TYPE_STRING)
        CWSH_THROW("Invalid Type for Operator.");

      break;

    case CWSH_EXPR_OPERATOR_TYPE_LESS:
    case CWSH_EXPR_OPERATOR_TYPE_LESS_OR_EQUAL:
    case CWSH_EXPR_OPERATOR_TYPE_GREATER:
    case CWSH_EXPR_OPERATOR_TYPE_GREATER_OR_EQUAL:
    case CWSH_EXPR_OPERATOR_TYPE_EQUAL:
    case CWSH_EXPR_OPERATOR_TYPE_NOT_EQUAL:
      if (type1 != type2)
        CWSH_THROW("Invalid Type Mix.");

      break;

    default:
      break;
  }

  int integer = 0;

  switch (opr->getType()) {
    case CWSH_EXPR_OPERATOR_TYPE_PLUS:
      integer = integer1 + integer2;

      break;
    case CWSH_EXPR_OPERATOR_TYPE_MINUS:
      integer = integer1 - integer2;

      break;
    case CWSH_EXPR_OPERATOR_TYPE_TIMES:
      integer = integer1 * integer2;

      break;
    case CWSH_EXPR_OPERATOR_TYPE_DIVIDE:
      if (integer2 == 0)
        CWSH_THROW("Divide By Zero.");

      integer = integer1 / integer2;

      break;
    case CWSH_EXPR_OPERATOR_TYPE_MODULUS:
      if (integer2 == 0)
        CWSH_THROW("Divide By Zero.");

      integer = integer1 % integer2;

      break;
    case CWSH_EXPR_OPERATOR_TYPE_LESS:
      if (type1 == CWSH_EXPR_PROCESS_VALUE_TYPE_INTEGER)
        integer = (integer1 < integer2);
      else
        integer = (value1 < value2);

      break;
    case CWSH_EXPR_OPERATOR_TYPE_LESS_OR_EQUAL:
      if (type1 == CWSH_EXPR_PROCESS_VALUE_TYPE_INTEGER)
        integer = (integer1 <= integer2);
      else
        integer = (value1 <= value2);

      break;
    case CWSH_EXPR_OPERATOR_TYPE_GREATER:
      if (type1 == CWSH_EXPR_PROCESS_VALUE_TYPE_INTEGER)
        integer = (integer1 > integer2);
      else
        integer = (value1 > value2);

      break;
    case CWSH_EXPR_OPERATOR_TYPE_GREATER_OR_EQUAL:
      if (type1 == CWSH_EXPR_PROCESS_VALUE_TYPE_INTEGER)
        integer = (integer1 >= integer2);
      else
        integer = (value1 >= value2);

      break;
    case CWSH_EXPR_OPERATOR_TYPE_EQUAL:
      if (type1 == CWSH_EXPR_PROCESS_VALUE_TYPE_INTEGER)
        integer = (integer1 == integer2);
      else
        integer = (value1 == value2);

      break;
    case CWSH_EXPR_OPERATOR_TYPE_NOT_EQUAL:
      if (type1 == CWSH_EXPR_PROCESS_VALUE_TYPE_INTEGER)
        integer = (integer1 != integer2);
      else
        integer = (value1 != value2);

      break;
    case CWSH_EXPR_OPERATOR_TYPE_MATCH_EQUAL: {
      CwshWildCard wildcard(value2);

      integer = wildcard.checkMatch(value1);

      break;
    }
    case CWSH_EXPR_OPERATOR_TYPE_NO_MATCH_EQUAL: {
      CwshWildCard wildcard(value2);

      integer = ! wildcard.checkMatch(value1);

      break;
    }
    case CWSH_EXPR_OPERATOR_TYPE_LOGICAL_AND:
      integer = (integer1 && integer2);

      break;
    case CWSH_EXPR_OPERATOR_TYPE_LOGICAL_OR:
      integer = (integer1 || integer2);

      break;
    case CWSH_EXPR_OPERATOR_TYPE_BIT_AND:
      integer = (integer1 & integer2);

      break;
    case CWSH_EXPR_OPERATOR_TYPE_BIT_OR:
      integer = (integer1 | integer2);

      break;
    case CWSH_EXPR_OPERATOR_TYPE_BIT_XOR:
      integer = (integer1 ^ integer2);

      break;
    case CWSH_EXPR_OPERATOR_TYPE_BIT_LSHIFT:
      integer = (integer1 << integer2);

      break;
    case CWSH_EXPR_OPERATOR_TYPE_BIT_RSHIFT:
      integer = (integer1 >> integer2);

      break;
    default:
      break;
  }

  string value = CStrUtil::toString(integer);

  return value;
}

CwshExprProcessValueType
CwshExprProcess::
getValueType(const string &value, int *integer)
{
  if (CStrUtil::isInteger(value)) {
    *integer = CStrUtil::toInteger(value);

    return CWSH_EXPR_PROCESS_VALUE_TYPE_INTEGER;
  }
  else
    return CWSH_EXPR_PROCESS_VALUE_TYPE_STRING;
}
