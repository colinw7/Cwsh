#include <CwshI.h>

CwshExprProcess::
CwshExprProcess()
{
}

std::string
CwshExprProcess::
process(CwshExprOperator *opr, const std::string &value)
{
  int integer;

  CwshExprProcessValueType type = getValueType(value, &integer);

  switch (opr->getType()) {
    case CwshExprOperatorType::UNARY_PLUS:
    case CwshExprOperatorType::UNARY_MINUS:
    case CwshExprOperatorType::LOGICAL_NOT:
    case CwshExprOperatorType::BIT_NOT:
      if (type != CwshExprProcessValueType::INTEGER)
        CWSH_THROW("Invalid Type for Operator.");

      break;
    case CwshExprOperatorType::IS_DIRECTORY:
    case CwshExprOperatorType::IS_FILE:
    case CwshExprOperatorType::IS_PLAIN:
    case CwshExprOperatorType::IS_OWNER:
    case CwshExprOperatorType::IS_READABLE:
    case CwshExprOperatorType::IS_WRITABLE:
    case CwshExprOperatorType::IS_EXECUTABLE:
    case CwshExprOperatorType::IS_ZERO:
      if (type != CwshExprProcessValueType::STRING)
        CWSH_THROW("Invalid Type for Operator.");

      break;
    default:
      break;
  }

  switch (opr->getType()) {
    case CwshExprOperatorType::UNARY_PLUS:
      break;
    case CwshExprOperatorType::UNARY_MINUS:
      integer = -integer;

      break;
    case CwshExprOperatorType::LOGICAL_NOT:
      if (! integer)
        integer = true;
      else
        integer = false;

      break;
    case CwshExprOperatorType::BIT_NOT:
      integer = ~integer;

      break;
    case CwshExprOperatorType::IS_DIRECTORY:
      integer = (CFile::exists(value) && CFile::isDirectory(value));

      break;
    case CwshExprOperatorType::IS_FILE:
      integer = CFile::exists(value);

      break;
    case CwshExprOperatorType::IS_PLAIN:
      integer = (CFile::exists(value) && CFile::isRegular(value));

      break;
    case CwshExprOperatorType::IS_OWNER:
      integer = (CFile::exists(value) && CFile::isOwner(value));

      break;
    case CwshExprOperatorType::IS_READABLE:
      integer = (CFile::exists(value) && CFile::isReadable(value));

      break;
    case CwshExprOperatorType::IS_WRITABLE:
      integer = (CFile::exists(value) && CFile::isWritable(value));

      break;
    case CwshExprOperatorType::IS_EXECUTABLE:
      integer = (CFile::exists(value) && CFile::isExecutable(value));

      break;
    case CwshExprOperatorType::IS_ZERO:
      integer = (CFile::exists(value) && CFile::getSize(value) == 0);

      break;
    default:
      break;
  }

  std::string value1 = CStrUtil::toString(integer);

  return value1;
}

std::string
CwshExprProcess::
process(const std::string &value1, CwshExprOperator *opr, const std::string &value2)
{
  int integer1;
  int integer2;

  CwshExprProcessValueType type1 = getValueType(value1, &integer1);
  CwshExprProcessValueType type2 = getValueType(value2, &integer2);

  switch (opr->getType()) {
    case CwshExprOperatorType::PLUS:
    case CwshExprOperatorType::MINUS:
    case CwshExprOperatorType::TIMES:
    case CwshExprOperatorType::DIVIDE:
    case CwshExprOperatorType::MODULUS:
    case CwshExprOperatorType::LOGICAL_AND:
    case CwshExprOperatorType::LOGICAL_OR:
    case CwshExprOperatorType::BIT_AND:
    case CwshExprOperatorType::BIT_OR:
    case CwshExprOperatorType::BIT_XOR:
    case CwshExprOperatorType::BIT_LSHIFT:
    case CwshExprOperatorType::BIT_RSHIFT:
      if (type1 != CwshExprProcessValueType::INTEGER ||
          type2 != CwshExprProcessValueType::INTEGER)
        CWSH_THROW("Invalid Type for Operator.");

      break;

    case CwshExprOperatorType::MATCH_EQUAL:
    case CwshExprOperatorType::NO_MATCH_EQUAL:
      if (type1 != CwshExprProcessValueType::STRING ||
          type2 != CwshExprProcessValueType::STRING)
        CWSH_THROW("Invalid Type for Operator.");

      break;

    case CwshExprOperatorType::LESS:
    case CwshExprOperatorType::LESS_OR_EQUAL:
    case CwshExprOperatorType::GREATER:
    case CwshExprOperatorType::GREATER_OR_EQUAL:
    case CwshExprOperatorType::EQUAL:
    case CwshExprOperatorType::NOT_EQUAL:
      if (type1 != type2)
        CWSH_THROW("Invalid Type Mix.");

      break;

    default:
      break;
  }

  int integer = 0;

  switch (opr->getType()) {
    case CwshExprOperatorType::PLUS:
      integer = integer1 + integer2;

      break;
    case CwshExprOperatorType::MINUS:
      integer = integer1 - integer2;

      break;
    case CwshExprOperatorType::TIMES:
      integer = integer1 * integer2;

      break;
    case CwshExprOperatorType::DIVIDE:
      if (integer2 == 0)
        CWSH_THROW("Divide By Zero.");

      integer = integer1 / integer2;

      break;
    case CwshExprOperatorType::MODULUS:
      if (integer2 == 0)
        CWSH_THROW("Divide By Zero.");

      integer = integer1 % integer2;

      break;
    case CwshExprOperatorType::LESS:
      if (type1 == CwshExprProcessValueType::INTEGER)
        integer = (integer1 < integer2);
      else
        integer = (value1 < value2);

      break;
    case CwshExprOperatorType::LESS_OR_EQUAL:
      if (type1 == CwshExprProcessValueType::INTEGER)
        integer = (integer1 <= integer2);
      else
        integer = (value1 <= value2);

      break;
    case CwshExprOperatorType::GREATER:
      if (type1 == CwshExprProcessValueType::INTEGER)
        integer = (integer1 > integer2);
      else
        integer = (value1 > value2);

      break;
    case CwshExprOperatorType::GREATER_OR_EQUAL:
      if (type1 == CwshExprProcessValueType::INTEGER)
        integer = (integer1 >= integer2);
      else
        integer = (value1 >= value2);

      break;
    case CwshExprOperatorType::EQUAL:
      if (type1 == CwshExprProcessValueType::INTEGER)
        integer = (integer1 == integer2);
      else
        integer = (value1 == value2);

      break;
    case CwshExprOperatorType::NOT_EQUAL:
      if (type1 == CwshExprProcessValueType::INTEGER)
        integer = (integer1 != integer2);
      else
        integer = (value1 != value2);

      break;
    case CwshExprOperatorType::MATCH_EQUAL: {
      CwshWildCard wildcard(value2);

      integer = wildcard.checkMatch(value1);

      break;
    }
    case CwshExprOperatorType::NO_MATCH_EQUAL: {
      CwshWildCard wildcard(value2);

      integer = ! wildcard.checkMatch(value1);

      break;
    }
    case CwshExprOperatorType::LOGICAL_AND:
      integer = (integer1 && integer2);

      break;
    case CwshExprOperatorType::LOGICAL_OR:
      integer = (integer1 || integer2);

      break;
    case CwshExprOperatorType::BIT_AND:
      integer = (integer1 & integer2);

      break;
    case CwshExprOperatorType::BIT_OR:
      integer = (integer1 | integer2);

      break;
    case CwshExprOperatorType::BIT_XOR:
      integer = (integer1 ^ integer2);

      break;
    case CwshExprOperatorType::BIT_LSHIFT:
      integer = (integer1 << integer2);

      break;
    case CwshExprOperatorType::BIT_RSHIFT:
      integer = (integer1 >> integer2);

      break;
    default:
      break;
  }

  std::string value = CStrUtil::toString(integer);

  return value;
}

CwshExprProcessValueType
CwshExprProcess::
getValueType(const std::string &value, int *integer)
{
  if (CStrUtil::isInteger(value)) {
    *integer = int(CStrUtil::toInteger(value));

    return CwshExprProcessValueType::INTEGER;
  }
  else
    return CwshExprProcessValueType::STRING;
}
