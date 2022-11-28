#include <CwshI.h>

namespace Cwsh {

/*
 *   Operator Token        | Precedence   | Associativity
 * ------------------------+--------------+--------------
 *                         |              |
 *  (   )                  |  12          |  L -> R
 *  !   ~   +   -          |  11 (unary)  |  R -> L
 *  -d  -e  -f  -o  -r  -w |  11 (unary)  |  R -> L
 *  -x  -z                 |  11 (unary)  |  R -> L
 *  *   /   %              |  10          |  L -> R
 *  +   -                  |   9          |  L -> R
 *  <<  >>                 |   8          |  L -> R
 *  <   <=  >   >=         |   7          |  L -> R
 *  ==  !=                 |   6          |  L -> R
 *  &                      |   5          |  L -> R
 *  ^                      |   4          |  L -> R
 *  |                      |   3          |  L -> R
 *  &&                     |   2          |  L -> R
 *  ||                     |   1          |  L -> R
 *
 */

ExprOperator
ExprOperator::
operators_[] = {
  ExprOperator(ExprOperatorType::OPEN_BRACKET    , "("  , 12, true , true , false),
  ExprOperator(ExprOperatorType::CLOSE_BRACKET   , ")"  , 12, true , true , false),
  ExprOperator(ExprOperatorType::UNARY_PLUS      , "+"  , 11, false, false, true ),
  ExprOperator(ExprOperatorType::UNARY_MINUS     , "-"  , 11, false, false, true ),
  ExprOperator(ExprOperatorType::LOGICAL_NOT     , "!"  , 11, false, false, true ),
  ExprOperator(ExprOperatorType::BIT_NOT         , "~"  , 11, false, false, true ),
  ExprOperator(ExprOperatorType::IS_DIRECTORY    , "-d" , 11, false, false, true ),
  ExprOperator(ExprOperatorType::IS_FILE         , "-e" , 11, false, false, true ),
  ExprOperator(ExprOperatorType::IS_PLAIN        , "-f" , 11, false, false, true ),
  ExprOperator(ExprOperatorType::IS_OWNER        , "-o" , 11, false, false, true ),
  ExprOperator(ExprOperatorType::IS_READABLE     , "-r" , 11, false, false, true ),
  ExprOperator(ExprOperatorType::IS_WRITABLE     , "-w" , 11, false, false, true ),
  ExprOperator(ExprOperatorType::IS_EXECUTABLE   , "-x" , 11, false, false, true ),
  ExprOperator(ExprOperatorType::IS_ZERO         , "-z" , 11, false, false, true ),
  ExprOperator(ExprOperatorType::PLUS            , "+"  ,  9, true , false, false),
  ExprOperator(ExprOperatorType::MINUS           , "-"  ,  9, true , false, false),
  ExprOperator(ExprOperatorType::TIMES           , "*"  , 10, true , false, false),
  ExprOperator(ExprOperatorType::DIVIDE          , "/"  , 10, true , false, false),
  ExprOperator(ExprOperatorType::MODULUS         , "%"  , 10, true , false, false),
  ExprOperator(ExprOperatorType::LESS            , "<"  ,  7, true , false, false),
  ExprOperator(ExprOperatorType::LESS_OR_EQUAL   , "<=" ,  7, true , false, false),
  ExprOperator(ExprOperatorType::GREATER         , ">"  ,  7, true , false, false),
  ExprOperator(ExprOperatorType::GREATER_OR_EQUAL, ">=" ,  7, true , false, false),
  ExprOperator(ExprOperatorType::EQUAL           , "==" ,  6, true , false, false),
  ExprOperator(ExprOperatorType::NOT_EQUAL       , "!=" ,  6, true , false, false),
  ExprOperator(ExprOperatorType::MATCH_EQUAL     , "=~" ,  6, true , false, false),
  ExprOperator(ExprOperatorType::NO_MATCH_EQUAL  , "!~" ,  6, true , false, false),
  ExprOperator(ExprOperatorType::LOGICAL_AND     , "&&" ,  2, true , false, false),
  ExprOperator(ExprOperatorType::LOGICAL_OR      , "||" ,  1, true , false, false),
  ExprOperator(ExprOperatorType::BIT_AND         , "&"  ,  5, true , false, false),
  ExprOperator(ExprOperatorType::BIT_OR          , "|"  ,  3, true , false, false),
  ExprOperator(ExprOperatorType::BIT_XOR         , "^"  ,  4, true , false, false),
  ExprOperator(ExprOperatorType::BIT_LSHIFT      , "<<" ,  8, true , false, false),
  ExprOperator(ExprOperatorType::BIT_RSHIFT      , ">>" ,  8, true , false, false),
};

int ExprOperator::numOperators_ = sizeof(ExprOperator::operators_)/sizeof(ExprOperator);

ExprOperator::
ExprOperator(ExprOperatorType type, const std::string &token, uint precedence,
             bool associateLtoR, bool punctuation, bool unary) :
 type_(type), token_(token), precedence_(precedence), associateLtoR_(associateLtoR),
 punctuation_(punctuation), unary_(unary)
{
}

ExprOperator *
ExprOperator::
lookup(ExprOperatorType type)
{
  for (int i = 0; i < numOperators_; i++)
    if (operators_[i].type_ == type)
      return &operators_[i];

  CWSH_THROW("Invalid Operator Type");
}

}
