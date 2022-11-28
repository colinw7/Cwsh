#include <CwshI.h>

namespace Cwsh {

ExprProcess::
ExprProcess()
{
}

std::string
ExprProcess::
process(ExprOperator *opr, const std::string &value)
{
  int integer;

  auto type = getValueType(value, &integer);

  switch (opr->getType()) {
    case ExprOperatorType::UNARY_PLUS:
    case ExprOperatorType::UNARY_MINUS:
    case ExprOperatorType::LOGICAL_NOT:
    case ExprOperatorType::BIT_NOT:
      if (type != ExprProcessValueType::INTEGER)
        CWSH_THROW("Invalid Type for Operator.");

      break;
    case ExprOperatorType::IS_DIRECTORY:
    case ExprOperatorType::IS_FILE:
    case ExprOperatorType::IS_PLAIN:
    case ExprOperatorType::IS_OWNER:
    case ExprOperatorType::IS_READABLE:
    case ExprOperatorType::IS_WRITABLE:
    case ExprOperatorType::IS_EXECUTABLE:
    case ExprOperatorType::IS_ZERO:
      if (type != ExprProcessValueType::STRING)
        CWSH_THROW("Invalid Type for Operator.");

      break;
    default:
      break;
  }

  switch (opr->getType()) {
    case ExprOperatorType::UNARY_PLUS:
      break;
    case ExprOperatorType::UNARY_MINUS:
      integer = -integer;

      break;
    case ExprOperatorType::LOGICAL_NOT:
      if (! integer)
        integer = true;
      else
        integer = false;

      break;
    case ExprOperatorType::BIT_NOT:
      integer = ~integer;

      break;
    case ExprOperatorType::IS_DIRECTORY:
      integer = (CFile::exists(value) && CFile::isDirectory(value));

      break;
    case ExprOperatorType::IS_FILE:
      integer = CFile::exists(value);

      break;
    case ExprOperatorType::IS_PLAIN:
      integer = (CFile::exists(value) && CFile::isRegular(value));

      break;
    case ExprOperatorType::IS_OWNER:
      integer = (CFile::exists(value) && CFile::isOwner(value));

      break;
    case ExprOperatorType::IS_READABLE:
      integer = (CFile::exists(value) && CFile::isReadable(value));

      break;
    case ExprOperatorType::IS_WRITABLE:
      integer = (CFile::exists(value) && CFile::isWritable(value));

      break;
    case ExprOperatorType::IS_EXECUTABLE:
      integer = (CFile::exists(value) && CFile::isExecutable(value));

      break;
    case ExprOperatorType::IS_ZERO:
      integer = (CFile::exists(value) && CFile::getSize(value) == 0);

      break;
    default:
      break;
  }

  std::string value1 = CStrUtil::toString(integer);

  return value1;
}

std::string
ExprProcess::
process(const std::string &value1, ExprOperator *opr, const std::string &value2)
{
  int integer1;
  int integer2;

  auto type1 = getValueType(value1, &integer1);
  auto type2 = getValueType(value2, &integer2);

  switch (opr->getType()) {
    case ExprOperatorType::PLUS:
    case ExprOperatorType::MINUS:
    case ExprOperatorType::TIMES:
    case ExprOperatorType::DIVIDE:
    case ExprOperatorType::MODULUS:
    case ExprOperatorType::LOGICAL_AND:
    case ExprOperatorType::LOGICAL_OR:
    case ExprOperatorType::BIT_AND:
    case ExprOperatorType::BIT_OR:
    case ExprOperatorType::BIT_XOR:
    case ExprOperatorType::BIT_LSHIFT:
    case ExprOperatorType::BIT_RSHIFT:
      if (type1 != ExprProcessValueType::INTEGER || type2 != ExprProcessValueType::INTEGER)
        CWSH_THROW("Invalid Type for Operator.");

      break;

    case ExprOperatorType::MATCH_EQUAL:
    case ExprOperatorType::NO_MATCH_EQUAL:
      if (type1 != ExprProcessValueType::STRING || type2 != ExprProcessValueType::STRING)
        CWSH_THROW("Invalid Type for Operator.");

      break;

    case ExprOperatorType::LESS:
    case ExprOperatorType::LESS_OR_EQUAL:
    case ExprOperatorType::GREATER:
    case ExprOperatorType::GREATER_OR_EQUAL:
    case ExprOperatorType::EQUAL:
    case ExprOperatorType::NOT_EQUAL:
      if (type1 != type2)
        CWSH_THROW("Invalid Type Mix.");

      break;

    default:
      break;
  }

  int integer = 0;

  switch (opr->getType()) {
    case ExprOperatorType::PLUS:
      integer = integer1 + integer2;

      break;
    case ExprOperatorType::MINUS:
      integer = integer1 - integer2;

      break;
    case ExprOperatorType::TIMES:
      integer = integer1 * integer2;

      break;
    case ExprOperatorType::DIVIDE:
      if (integer2 == 0)
        CWSH_THROW("Divide By Zero.");

      integer = integer1 / integer2;

      break;
    case ExprOperatorType::MODULUS:
      if (integer2 == 0)
        CWSH_THROW("Divide By Zero.");

      integer = integer1 % integer2;

      break;
    case ExprOperatorType::LESS:
      if (type1 == ExprProcessValueType::INTEGER)
        integer = (integer1 < integer2);
      else
        integer = (value1 < value2);

      break;
    case ExprOperatorType::LESS_OR_EQUAL:
      if (type1 == ExprProcessValueType::INTEGER)
        integer = (integer1 <= integer2);
      else
        integer = (value1 <= value2);

      break;
    case ExprOperatorType::GREATER:
      if (type1 == ExprProcessValueType::INTEGER)
        integer = (integer1 > integer2);
      else
        integer = (value1 > value2);

      break;
    case ExprOperatorType::GREATER_OR_EQUAL:
      if (type1 == ExprProcessValueType::INTEGER)
        integer = (integer1 >= integer2);
      else
        integer = (value1 >= value2);

      break;
    case ExprOperatorType::EQUAL:
      if (type1 == ExprProcessValueType::INTEGER)
        integer = (integer1 == integer2);
      else
        integer = (value1 == value2);

      break;
    case ExprOperatorType::NOT_EQUAL:
      if (type1 == ExprProcessValueType::INTEGER)
        integer = (integer1 != integer2);
      else
        integer = (value1 != value2);

      break;
    case ExprOperatorType::MATCH_EQUAL: {
      WildCard wildcard(value2);

      integer = wildcard.checkMatch(value1);

      break;
    }
    case ExprOperatorType::NO_MATCH_EQUAL: {
      WildCard wildcard(value2);

      integer = ! wildcard.checkMatch(value1);

      break;
    }
    case ExprOperatorType::LOGICAL_AND:
      integer = (integer1 && integer2);

      break;
    case ExprOperatorType::LOGICAL_OR:
      integer = (integer1 || integer2);

      break;
    case ExprOperatorType::BIT_AND:
      integer = (integer1 & integer2);

      break;
    case ExprOperatorType::BIT_OR:
      integer = (integer1 | integer2);

      break;
    case ExprOperatorType::BIT_XOR:
      integer = (integer1 ^ integer2);

      break;
    case ExprOperatorType::BIT_LSHIFT:
      integer = (integer1 << integer2);

      break;
    case ExprOperatorType::BIT_RSHIFT:
      integer = (integer1 >> integer2);

      break;
    default:
      break;
  }

  std::string value = CStrUtil::toString(integer);

  return value;
}

ExprProcessValueType
ExprProcess::
getValueType(const std::string &value, int *integer)
{
  if (CStrUtil::isInteger(value)) {
    *integer = int(CStrUtil::toInteger(value));

    return ExprProcessValueType::INTEGER;
  }
  else
    return ExprProcessValueType::STRING;
}

}
