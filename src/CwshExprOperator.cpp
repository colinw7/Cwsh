#include "CwshI.h"

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
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_OPEN_BRACKET    , "("  ,
                   12, true , true , false),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_CLOSE_BRACKET   , ")"  ,
                   12, true , true , false),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_UNARY_PLUS      , "+"  ,
                   11, false, false, true ),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_UNARY_MINUS     , "-"  ,
                   11, false, false, true ),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_LOGICAL_NOT     , "!"  ,
                   11, false, false, true ),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_BIT_NOT         , "~"  ,
                   11, false, false, true ),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_IS_DIRECTORY    , "-d" ,
                   11, false, false, true ),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_IS_FILE         , "-e" ,
                   11, false, false, true ),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_IS_PLAIN        , "-f" ,
                   11, false, false, true ),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_IS_OWNER        , "-o" ,
                   11, false, false, true ),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_IS_READABLE     , "-r" ,
                   11, false, false, true ),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_IS_WRITABLE     , "-w" ,
                   11, false, false, true ),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_IS_EXECUTABLE   , "-x" ,
                   11, false, false, true ),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_IS_ZERO         , "-z" ,
                   11, false, false, true ),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_PLUS            , "+"  ,
                    9, true , false, false),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_MINUS           , "-"  ,
                    9, true , false, false),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_TIMES           , "*"  ,
                   10, true , false, false),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_DIVIDE          , "/"  ,
                   10, true , false, false),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_MODULUS         , "%"  ,
                   10, true , false, false),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_LESS            , "<"  ,
                    7, true , false, false),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_LESS_OR_EQUAL   , "<=" ,
                    7, true , false, false),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_GREATER         , ">"  ,
                    7, true , false, false),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_GREATER_OR_EQUAL, ">=" ,
                    7, true , false, false),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_EQUAL           , "==" ,
                    6, true , false, false),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_NOT_EQUAL       , "!=" ,
                    6, true , false, false),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_MATCH_EQUAL     , "=~" ,
                    6, true , false, false),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_NO_MATCH_EQUAL  , "!~" ,
                    6, true , false, false),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_LOGICAL_AND     , "&&" ,
                    2, true , false, false),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_LOGICAL_OR      , "||" ,
                    1, true , false, false),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_BIT_AND         , "&"  ,
                    5, true , false, false),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_BIT_OR          , "|"  ,
                    3, true , false, false),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_BIT_XOR         , "^"  ,
                    4, true , false, false),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_BIT_LSHIFT      , "<<" ,
                    8, true , false, false),
  CwshExprOperator(CWSH_EXPR_OPERATOR_TYPE_BIT_RSHIFT      , ">>" ,
                    8, true , false, false),
};

int CwshExprOperator::num_operators_ =
  sizeof(CwshExprOperator::operators_)/sizeof(CwshExprOperator);

CwshExprOperator::
CwshExprOperator(CwshExprOperatorType type, const string &token,
                 uint precedence, bool associate_l_to_r,
                 bool punctuation, bool unary) :
 type_(type), token_(token), precedence_(precedence),
 associate_l_to_r_(associate_l_to_r), punctuation_(punctuation),
 unary_(unary)
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
