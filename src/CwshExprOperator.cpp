#include <CwshI.h>

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

CwshExprOperator
CwshExprOperator::
operators_[] = {
  CwshExprOperator(CwshExprOperatorType::OPEN_BRACKET    , "("  , 12, true , true , false),
  CwshExprOperator(CwshExprOperatorType::CLOSE_BRACKET   , ")"  , 12, true , true , false),
  CwshExprOperator(CwshExprOperatorType::UNARY_PLUS      , "+"  , 11, false, false, true ),
  CwshExprOperator(CwshExprOperatorType::UNARY_MINUS     , "-"  , 11, false, false, true ),
  CwshExprOperator(CwshExprOperatorType::LOGICAL_NOT     , "!"  , 11, false, false, true ),
  CwshExprOperator(CwshExprOperatorType::BIT_NOT         , "~"  , 11, false, false, true ),
  CwshExprOperator(CwshExprOperatorType::IS_DIRECTORY    , "-d" , 11, false, false, true ),
  CwshExprOperator(CwshExprOperatorType::IS_FILE         , "-e" , 11, false, false, true ),
  CwshExprOperator(CwshExprOperatorType::IS_PLAIN        , "-f" , 11, false, false, true ),
  CwshExprOperator(CwshExprOperatorType::IS_OWNER        , "-o" , 11, false, false, true ),
  CwshExprOperator(CwshExprOperatorType::IS_READABLE     , "-r" , 11, false, false, true ),
  CwshExprOperator(CwshExprOperatorType::IS_WRITABLE     , "-w" , 11, false, false, true ),
  CwshExprOperator(CwshExprOperatorType::IS_EXECUTABLE   , "-x" , 11, false, false, true ),
  CwshExprOperator(CwshExprOperatorType::IS_ZERO         , "-z" , 11, false, false, true ),
  CwshExprOperator(CwshExprOperatorType::PLUS            , "+"  ,  9, true , false, false),
  CwshExprOperator(CwshExprOperatorType::MINUS           , "-"  ,  9, true , false, false),
  CwshExprOperator(CwshExprOperatorType::TIMES           , "*"  , 10, true , false, false),
  CwshExprOperator(CwshExprOperatorType::DIVIDE          , "/"  , 10, true , false, false),
  CwshExprOperator(CwshExprOperatorType::MODULUS         , "%"  , 10, true , false, false),
  CwshExprOperator(CwshExprOperatorType::LESS            , "<"  ,  7, true , false, false),
  CwshExprOperator(CwshExprOperatorType::LESS_OR_EQUAL   , "<=" ,  7, true , false, false),
  CwshExprOperator(CwshExprOperatorType::GREATER         , ">"  ,  7, true , false, false),
  CwshExprOperator(CwshExprOperatorType::GREATER_OR_EQUAL, ">=" ,  7, true , false, false),
  CwshExprOperator(CwshExprOperatorType::EQUAL           , "==" ,  6, true , false, false),
  CwshExprOperator(CwshExprOperatorType::NOT_EQUAL       , "!=" ,  6, true , false, false),
  CwshExprOperator(CwshExprOperatorType::MATCH_EQUAL     , "=~" ,  6, true , false, false),
  CwshExprOperator(CwshExprOperatorType::NO_MATCH_EQUAL  , "!~" ,  6, true , false, false),
  CwshExprOperator(CwshExprOperatorType::LOGICAL_AND     , "&&" ,  2, true , false, false),
  CwshExprOperator(CwshExprOperatorType::LOGICAL_OR      , "||" ,  1, true , false, false),
  CwshExprOperator(CwshExprOperatorType::BIT_AND         , "&"  ,  5, true , false, false),
  CwshExprOperator(CwshExprOperatorType::BIT_OR          , "|"  ,  3, true , false, false),
  CwshExprOperator(CwshExprOperatorType::BIT_XOR         , "^"  ,  4, true , false, false),
  CwshExprOperator(CwshExprOperatorType::BIT_LSHIFT      , "<<" ,  8, true , false, false),
  CwshExprOperator(CwshExprOperatorType::BIT_RSHIFT      , ">>" ,  8, true , false, false),
};

int CwshExprOperator::num_operators_ =
  sizeof(CwshExprOperator::operators_)/sizeof(CwshExprOperator);

CwshExprOperator::
CwshExprOperator(CwshExprOperatorType type, const std::string &token, uint precedence,
                 bool associate_l_to_r, bool punctuation, bool unary) :
 type_(type), token_(token), precedence_(precedence), associate_l_to_r_(associate_l_to_r),
 punctuation_(punctuation), unary_(unary)
{
}

CwshExprOperator *
CwshExprOperator::
lookup(CwshExprOperatorType type)
{
  for (int i = 0; i < num_operators_; i++)
    if (operators_[i].type_ == type)
      return &operators_[i];

  CWSH_THROW("Invalid Operator Type");
}
