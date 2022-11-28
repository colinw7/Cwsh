#include <CwshI.h>

namespace Cwsh {

ExprStackStack::
ExprStackStack()
{
  current_ = std::make_shared<ExprStack>();
}

ExprStackStack::
~ExprStackStack()
{
}

void
ExprStackStack::
push(const std::string &value)
{
  if (current_)
    current_->push(value);
}

void
ExprStackStack::
push(ExprOperator *opr)
{
  if (current_)
    current_->push(opr);
}

bool
ExprStackStack::
pop(std::string &value)
{
  if (current_) {
    bool flag = current_->pop(value);

    return flag;
  }
  else
    return false;
}

bool
ExprStackStack::
pop(ExprOperator **opr)
{
  if (current_) {
    bool flag = current_->pop(opr);

    return flag;
  }
  else
    return false;
}

void
ExprStackStack::
remove(ExprStackNode *stackNode)
{
  if (current_)
    current_->remove(stackNode);
}

void
ExprStackStack::
restart(bool flag)
{
  if (flag)
    stacks_.clear();

  if (current_)
    current_->restart(flag);
}

ExprStack *
ExprStackStack::
start()
{
  if (current_)
    stacks_.push_back(current_);

  current_ = std::make_shared<ExprStack>();

  return current_.get();
}

void
ExprStackStack::
end()
{
  current_ = stacks_[stacks_.size() - 1];

  stacks_.pop_back();
}

void
ExprStackStack::
startTemporary()
{
  start();
}

void
ExprStackStack::
endTemporary()
{
  end();
}

ExprStack *
ExprStackStack::
getCurrent() const
{
  return current_.get();
}

#if 0
void
ExprStackStack::
setCurrent(ExprStack *stack)
{
  current_ = stack;
}
#endif

void
ExprStackStack::
toStart()
{
  if (current_)
    current_->toStart();
}

void
ExprStackStack::
toEnd()
{
  if (current_)
    current_->toEnd();
}

void
ExprStackStack::
setInBrackets(bool flag)
{
  if (current_)
    current_->setInBrackets(flag);
}

bool
ExprStackStack::
getInBrackets()
{
  if (current_)
    return current_->getInBrackets();
  else
    return false;
}

void
ExprStackStack::
signalRestart()
{
  if (current_)
    current_->signalRestart();
}

bool
ExprStackStack::
checkRestart()
{
  if (current_)
    return current_->checkRestart();
  else
    return false;
}

ExprStackNode *
ExprStackStack::
getCurrentNode()
{
  if (current_)
    return current_->getCurrent();

  return nullptr;
}

void
ExprStackStack::
toNext()
{
  if (current_)
    current_->toNext();
}

void
ExprStackStack::
toPrev()
{
  if (current_)
    current_->toPrev();
}

void
ExprStackStack::
setLastOperator()
{
  if (current_)
    current_->setLastOperator();
}

ExprOperator *
ExprStackStack::
getLastOperator()
{
  if (current_)
    return current_->getLastOperator();

  return nullptr;
}

//----------

ExprStack::
ExprStack()
{
  pstackNode_ = stackNodes_.begin();
}

ExprStack::
~ExprStack()
{
  for (auto &stackNode : stackNodes_)
    delete stackNode;
}

void
ExprStack::
push(const std::string &value)
{
  if (debug_) {
    dumpStack();
    std::cerr << "Push Value: " << value << "\n";
  }

  auto *stackNode = new ExprStackNode(value);

  push(stackNode);

  if (debug_)
    dumpStack();
}

void
ExprStack::
push(ExprOperator *opr)
{
  if (debug_) {
    dumpStack();
    std::cerr << "Push Opr: " << opr->getToken() << "\n";
  }

  auto *stackNode = new ExprStackNode(opr);

  push(stackNode);

  if (debug_)
    dumpStack();
}

void
ExprStack::
push(ExprStackNode *stackNode)
{
  if (pstackNode_ != stackNodes_.end()) {
    pstackNode_++;

    pstackNode_ = stackNodes_.insert(pstackNode_, stackNode);
  }
  else {
    stackNodes_.push_front(stackNode);

    pstackNode_ = stackNodes_.begin();
  }
}

bool
ExprStack::
pop(std::string &value)
{
  if (debug_)
    dumpStack();

  if (pstackNode_ != stackNodes_.end() &&
      (*pstackNode_)->getType() == ExprStackNodeType::VALUE) {
    value = (*pstackNode_)->getValue();

    remove(*pstackNode_);

    if (debug_) {
      std::cerr << "Pop Value: " << value << "\n";
      dumpStack();
    }

    return true;
  }
  else
    return false;
}

bool
ExprStack::
pop(ExprOperator **opr)
{
  if (debug_)
    dumpStack();

  if (pstackNode_ != stackNodes_.end() &&
      (*pstackNode_)->getType() == ExprStackNodeType::OPERATOR) {
    *opr = (*pstackNode_)->getOperator();

    remove(*pstackNode_);

    if (debug_) {
      std::cerr << "Pop Opr: " << (*opr)->getToken() << "\n";
      dumpStack();
    }

    return true;
  }
  else
    return false;
}

void
ExprStack::
remove(ExprStackNode *stackNode)
{
  pstackNode_--;

  stackNodes_.remove(stackNode);

  delete stackNode;
}

void
ExprStack::
restart(bool flag)
{
  restart_   = flag;
  inBracket_ = false;
  lastOpr_   = nullptr;

  for (auto &stackNode : stackNodes_)
    delete stackNode;

  stackNodes_.clear();

  pstackNode_ = stackNodes_.begin();
}

void
ExprStack::
toStart()
{
  restart_   = false;
  inBracket_ = false;
  lastOpr_   = nullptr;

  pstackNode_ = stackNodes_.begin();
}

void
ExprStack::
toEnd()
{
  restart_   = false;
  inBracket_ = false;

  if (stackNodes_.size() > 0) {
    pstackNode_ = stackNodes_.end();

    --pstackNode_;
  }
}

void
ExprStack::
setInBrackets(bool flag)
{
  inBracket_ = flag;
}

bool
ExprStack::
getInBrackets()
{
  return inBracket_;
}

void
ExprStack::
signalRestart()
{
  restart_ = true;
}

bool
ExprStack::
checkRestart()
{
  return restart_;
}

ExprStackNode *
ExprStack::
getCurrent()
{
  if (pstackNode_ != stackNodes_.end())
    return *pstackNode_;

  return nullptr;
}

void
ExprStack::
toNext()
{
  if (pstackNode_ != stackNodes_.end())
    ++pstackNode_;
}

void
ExprStack::
toPrev()
{
  if (pstackNode_ != stackNodes_.begin())
    --pstackNode_;
}

void
ExprStack::
setLastOperator()
{
  lastOpr_ = nullptr;

  auto pstackNode1 = stackNodes_.begin();
  auto pstackNode2 = stackNodes_.end  ();

  for ( ; pstackNode1 != pstackNode2; ++pstackNode1) {
    if (pstackNode1 == pstackNode_)
      break;

    if ((*pstackNode1)->getType() == ExprStackNodeType::OPERATOR) {
      if ((*pstackNode1)->getOperator()->isPunctuation())
        lastOpr_ = nullptr;
      else
        lastOpr_ = (*pstackNode1)->getOperator();
    }
  }
}

ExprOperator *
ExprStack::
getLastOperator()
{
  setLastOperator();

  return lastOpr_;
}

void
ExprStack::
dumpStack()
{
  std::cerr << "Stack: ";

  auto pstackNode1 = stackNodes_.begin();
  auto pstackNode2 = stackNodes_.end  ();

  for ( ; pstackNode1 != pstackNode2; ++pstackNode1) {
    (*pstackNode1)->dump();

    std::cerr << " ";
  }

  std::cerr << "\n";

  std::cerr << "Current: ";

  if (pstackNode_ != stackNodes_.end())
    (*pstackNode_)->dump();
  else
    std::cerr << "End";

  std::cerr << "\n";
}

ExprStackNode::
ExprStackNode(const std::string &value) :
 type_(ExprStackNodeType::VALUE), opr_(nullptr), value_(value)
{
}

ExprStackNode::
ExprStackNode(ExprOperator *opr) :
 type_(ExprStackNodeType::OPERATOR), opr_(opr), value_("")
{
}

ExprStackNode::
~ExprStackNode()
{
}

void
ExprStackNode::
print() const
{
  if (type_ == ExprStackNodeType::VALUE)
    std::cout << "Value " << value_ << "\n";
  else
    std::cout << "Operator " << opr_->getToken() << "\n";
}

void
ExprStackNode::
dump() const
{
  if (type_ == ExprStackNodeType::VALUE)
    std::cerr << "<value>" << value_;
  else
    std::cerr << "<opr>" << opr_->getToken();
}

}
