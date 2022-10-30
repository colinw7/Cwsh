#include <CwshI.h>

CwshExprEvaluate::
CwshExprEvaluate(Cwsh *cwsh, const std::string &expression) :
 cwsh_(cwsh), expression_(expression)
{
}

int
CwshExprEvaluate::
process()
{
  std::string value = evaluate();

  if (! CStrUtil::isInteger(value))
    CWSH_THROW("Invalid Expression '" + expression_ + "'");

  int integer = int(CStrUtil::toInteger(value));

  return integer;
}

std::string
CwshExprEvaluate::
evaluate()
{
  stack_ = new CwshExprStackStack();

  CwshExprParse parse(cwsh_);

  parse.stack(stack_, expression_);

  std::string value = evaluateStack();

  stack_ = nullptr;

  return value;
}

std::string
CwshExprEvaluate::
evaluateStack()
{
  stack_->toStart();

  std::string value = unstack();

  return value;
}

std::string
CwshExprEvaluate::
unstack()
{
  CwshExprStackNode *initial_stack_node = stack_->getCurrentNode();

  CwshExprStackNode *stack_node = initial_stack_node;

  if (! stack_node)
    CWSH_THROW("Null Expression.");

  bool end_expression = false;
  bool value_on_stack = false;

  while (! end_expression) {
    if      (stack_node->getType() == CwshExprStackNodeType::OPERATOR) {
      if      (stack_node->getOperator()->isUnary()) {
        stack_->toNext();

        value_on_stack = false;
      }
      else if (stack_node->getOperator()->isBinary()) {
        if (checkUnstack(stack_node->getOperator())) {
          stack_->toPrev();

          CwshExprOperator *last_opr = stack_->getLastOperator();

          if      (last_opr->isUnary()) {
            unstackUnary();

            value_on_stack = true;
          }
          else if (last_opr->isBinary()) {
            unstackBinary();

            value_on_stack = true;
          }
          else
            CWSH_THROW("Invalid Operator.");

          stack_->toNext();

          value_on_stack = false;
        }
        else {
          stack_->toNext();

          value_on_stack = false;
        }
      }
      else if (stack_node->getOperator()->getType() ==
                CwshExprOperatorType::OPEN_BRACKET) {
        unstackBracketed();

        value_on_stack = true;
      }
      else if (stack_node->getOperator()->isPunctuation())
        end_expression = true;
      else
        CWSH_THROW("Invalid Operator.");
    }
    else if (stack_node->getType() == CwshExprStackNodeType::VALUE) {
      stack_->toNext();

      value_on_stack = true;
    }
    else
      CWSH_THROW("Invalid Character.");

    stack_node = stack_->getCurrentNode();

    if (! stack_node)
      end_expression = true;
  }

  stack_->toPrev();

  if (initial_stack_node == stack_node)
    CWSH_THROW("Null Expression.");

  if (cwsh_->getDebug() && ! value_on_stack)
    std::cerr << "No value on stack\n";

  std::string value = backUnstack();

  return value;
}

std::string
CwshExprEvaluate::
backUnstack()
{
  CwshExprOperator *last_opr = stack_->getLastOperator();

  while (last_opr && (last_opr->isUnary() || last_opr->isBinary())) {
    if      (last_opr->isUnary())
      unstackUnary();
    else if (last_opr->isBinary())
      unstackBinary();

    last_opr = stack_->getLastOperator();
  }

  std::string value;

  if (! stack_->pop(value))
    CWSH_THROW("Undefined Value.");

  return value;
}

void
CwshExprEvaluate::
unstackUnary()
{
  std::string value1;

  if (! stack_->pop(value1))
    CWSH_THROW("Undefined Value.");

  CwshExprOperator *opr;

  if (! stack_->pop(&opr))
    CWSH_THROW("Undefined Operator.");

  CwshExprProcess process;

  std::string value = process.process(opr, value1);

  stack_->push(value);
}

void
CwshExprEvaluate::
unstackBinary()
{
  std::string value1;
  std::string value2;

  if (! stack_->pop(value2))
    CWSH_THROW("Undefined Value.");

  CwshExprOperator *opr;

  if (! stack_->pop(&opr))
    CWSH_THROW("Undefined Operator.");

  if (! stack_->pop(value1))
    CWSH_THROW("Undefined Value.");

  CwshExprProcess process;

  std::string value = process.process(value1, opr, value2);

  stack_->push(value);
}

void
CwshExprEvaluate::
unstackBracketed()
{
  stack_->toNext();

  std::string value = unstack();

  stack_->toNext();

  CwshExprOperator *opr;

  if (! stack_->pop(&opr))
    CWSH_THROW("Undefined Operator.");

  if (! stack_->pop(&opr))
    CWSH_THROW("Undefined Operator.");

  stack_->push(value);
}

void
CwshExprEvaluate::
skip()
{
  CwshExprStackNode *stack_node = stack_->getCurrentNode();

  if (! stack_node)
    CWSH_THROW("Null Expression.");

  bool end_expression = false;

  while (! end_expression) {
    if      (stack_node->getType() == CwshExprStackNodeType::OPERATOR) {
      if      (stack_node->getOperator()->isUnary()) {
        stack_->remove(stack_node);

        stack_->toNext();
      }
      else if (stack_node->getOperator()->isBinary()) {
        stack_->remove(stack_node);

        stack_->toNext();
      }
      else if (stack_node->getOperator()->getType() ==
                CwshExprOperatorType::OPEN_BRACKET) {
        stack_->remove(stack_node);

        stack_->toNext();

        skip();

        stack_node = stack_->getCurrentNode();

        while (stack_node->getType() != CwshExprStackNodeType::OPERATOR ||
               stack_node->getOperator()->getType() !=
                CwshExprOperatorType::CLOSE_BRACKET) {
          stack_->remove(stack_node);

          stack_->toNext();

          skip();

          stack_node = stack_->getCurrentNode();
        }

        stack_->remove(stack_node);

        stack_->toNext();
      }
      else if (stack_node->getOperator()->isPunctuation())
        end_expression = true;
      else
        CWSH_THROW("Invalid Operator.");
    }
    else if (stack_node->getType() == CwshExprStackNodeType::VALUE) {
      stack_->remove(stack_node);

      stack_->toNext();
    }
    else
      CWSH_THROW("Invalid Character.");

    stack_node = stack_->getCurrentNode();

    if (! stack_node)
      end_expression = true;
  }
}

bool
CwshExprEvaluate::
checkUnstack(CwshExprOperator *opr)
{
  CwshExprOperator *last_opr = stack_->getLastOperator();

  if (last_opr &&
      (last_opr->getPrecedence() > opr->getPrecedence() ||
       (last_opr->getPrecedence() == opr->getPrecedence() && opr->doesAssociateLtoR())))
    return true;
  else
    return false;
}
