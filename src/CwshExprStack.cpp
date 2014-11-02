#include <CwshI.h>

CwshExprStackStack::
CwshExprStackStack()
{
  current_ = new CwshExprStack();
}

CwshExprStackStack::
~CwshExprStackStack()
{
  std::for_each(stacks_.begin(), stacks_.end(), CDeletePointer());
}

void
CwshExprStackStack::
push(const string &value)
{
  if (current_ != NULL)
    current_->push(value);
}

void
CwshExprStackStack::
push(CwshExprOperator *opr)
{
  if (current_ != NULL)
    current_->push(opr);
}

bool
CwshExprStackStack::
pop(string &value)
{
  if (current_ != NULL) {
    bool flag = current_->pop(value);

    return flag;
  }
  else
    return false;
}

bool
CwshExprStackStack::
pop(CwshExprOperator **opr)
{
  if (current_ != NULL) {
    bool flag = current_->pop(opr);

    return flag;
  }
  else
    return false;
}

void
CwshExprStackStack::
remove(CwshExprStackNode *stack_node)
{
  if (current_ != NULL)
    current_->remove(stack_node);
}

void
CwshExprStackStack::
restart(bool flag)
{
  if (flag) {
    std::for_each(stacks_.begin(), stacks_.end(), CDeletePointer());

    stacks_.clear();
  }

  if (current_ != NULL)
    current_->restart(flag);
}

CwshExprStack *
CwshExprStackStack::
start()
{
  if (current_ != NULL) {
    stacks_.push_back(current_);

    current_.release();
  }

  current_ = new CwshExprStack();

  return current_;
}

void
CwshExprStackStack::
end()
{
  current_ = stacks_[stacks_.size() - 1];

  stacks_.pop_back();
}

void
CwshExprStackStack::
startTemporary()
{
  start();
}

void
CwshExprStackStack::
endTemporary()
{
  end();
}

CwshExprStack *
CwshExprStackStack::
getCurrent() const
{
  return current_;
}

void
CwshExprStackStack::
setCurrent(CwshExprStack *stack)
{
  current_ = stack;
}

void
CwshExprStackStack::
toStart()
{
  if (current_ != NULL)
    current_->toStart();
}

void
CwshExprStackStack::
toEnd()
{
  if (current_ != NULL)
    current_->toEnd();
}

void
CwshExprStackStack::
setInBrackets(bool flag)
{
  if (current_ != NULL)
    current_->setInBrackets(flag);
}

bool
CwshExprStackStack::
getInBrackets()
{
  if (current_ != NULL)
    return current_->getInBrackets();
  else
    return false;
}

void
CwshExprStackStack::
signalRestart()
{
  if (current_ != NULL)
    current_->signalRestart();
}

bool
CwshExprStackStack::
checkRestart()
{
  if (current_ != NULL)
    return current_->checkRestart();
  else
    return false;
}

CwshExprStackNode *
CwshExprStackStack::
getCurrentNode()
{
  if (current_ != NULL)
    return current_->getCurrent();
  else
    return NULL;
}

void
CwshExprStackStack::
toNext()
{
  if (current_ != NULL)
    current_->toNext();
}

void
CwshExprStackStack::
toPrev()
{
  if (current_ != NULL)
    current_->toPrev();
}

void
CwshExprStackStack::
setLastOperator()
{
  if (current_ != NULL)
    current_->setLastOperator();
}

CwshExprOperator *
CwshExprStackStack::
getLastOperator()
{
  if (current_ != NULL)
    return current_->getLastOperator();
  else
    return NULL;
}

//----------

CwshExprStack::
CwshExprStack()
{
  restart_     = false;
  in_bracket_  = false;
  last_opr_    = NULL;
  pstack_node_ = stack_nodes_.begin();
  debug_       = false;
}

CwshExprStack::
~CwshExprStack()
{
  std::for_each(stack_nodes_.begin(), stack_nodes_.end(), CDeletePointer());
}

void
CwshExprStack::
push(const string &value)
{
  if (debug_) {
    dumpStack();
    std::cerr << "Push Value: " << value << std::endl;
  }

  CwshExprStackNode *stack_node = new CwshExprStackNode(value);

  push(stack_node);

  if (debug_)
    dumpStack();
}

void
CwshExprStack::
push(CwshExprOperator *opr)
{
  if (debug_) {
    dumpStack();
    std::cerr << "Push Opr: " << opr->getToken() << std::endl;
  }

  CwshExprStackNode *stack_node = new CwshExprStackNode(opr);

  push(stack_node);

  if (debug_)
    dumpStack();
}

void
CwshExprStack::
push(CwshExprStackNode *stack_node)
{
  if (pstack_node_ != stack_nodes_.end()) {
    pstack_node_++;

    pstack_node_ = stack_nodes_.insert(pstack_node_, stack_node);
  }
  else {
    stack_nodes_.push_front(stack_node);

    pstack_node_ = stack_nodes_.begin();
  }
}

bool
CwshExprStack::
pop(string &value)
{
  if (debug_)
    dumpStack();

  if (pstack_node_ != stack_nodes_.end() &&
      (*pstack_node_)->getType() == CWSH_STACK_NODE_TYPE_VALUE) {
    value = (*pstack_node_)->getValue();

    remove(*pstack_node_);

    if (debug_) {
      std::cerr << "Pop Value: " << value << std::endl;
      dumpStack();
    }

    return true;
  }
  else
    return false;
}

bool
CwshExprStack::
pop(CwshExprOperator **opr)
{
  if (debug_)
    dumpStack();

  if (pstack_node_ != stack_nodes_.end() &&
      (*pstack_node_)->getType() == CWSH_STACK_NODE_TYPE_OPERATOR) {
    *opr = (*pstack_node_)->getOperator();

    remove(*pstack_node_);

    if (debug_) {
      std::cerr << "Pop Opr: " << (*opr)->getToken() << std::endl;
      dumpStack();
    }

    return true;
  }
  else
    return false;
}

void
CwshExprStack::
remove(CwshExprStackNode *stack_node)
{
  pstack_node_--;

  stack_nodes_.remove(stack_node);

  delete stack_node;
}

void
CwshExprStack::
restart(bool flag)
{
  restart_    = flag;
  in_bracket_ = false;
  last_opr_   = NULL;

  std::for_each(stack_nodes_.begin(), stack_nodes_.end(), CDeletePointer());

  stack_nodes_.clear();

  pstack_node_ = stack_nodes_.begin();
}

void
CwshExprStack::
toStart()
{
  restart_    = false;
  in_bracket_ = false;
  last_opr_   = NULL;

  pstack_node_ = stack_nodes_.begin();
}

void
CwshExprStack::
toEnd()
{
  restart_    = false;
  in_bracket_ = false;

  if (stack_nodes_.size() > 0) {
    pstack_node_ = stack_nodes_.end();

    --pstack_node_;
  }
}

void
CwshExprStack::
setInBrackets(bool flag)
{
  in_bracket_ = flag;
}

bool
CwshExprStack::
getInBrackets()
{
  return in_bracket_;
}

void
CwshExprStack::
signalRestart()
{
  restart_ = true;
}

bool
CwshExprStack::
checkRestart()
{
  return restart_;
}

CwshExprStackNode *
CwshExprStack::
getCurrent()
{
  if (pstack_node_ != stack_nodes_.end())
    return *pstack_node_;
  else
    return NULL;
}

void
CwshExprStack::
toNext()
{
  if (pstack_node_ != stack_nodes_.end())
    ++pstack_node_;
}

void
CwshExprStack::
toPrev()
{
  if (pstack_node_ != stack_nodes_.begin())
    --pstack_node_;
}

void
CwshExprStack::
setLastOperator()
{
  last_opr_ = NULL;

  CwshExprStackNodeList::iterator pstack_node1 = stack_nodes_.begin();
  CwshExprStackNodeList::iterator pstack_node2 = stack_nodes_.end  ();

  for ( ; pstack_node1 != pstack_node2; ++pstack_node1) {
    if (pstack_node1 == pstack_node_)
      break;

    if ((*pstack_node1)->getType() == CWSH_STACK_NODE_TYPE_OPERATOR) {
      if ((*pstack_node1)->getOperator()->isPunctuation())
        last_opr_ = NULL;
      else
        last_opr_ = (*pstack_node1)->getOperator();
    }
  }
}

CwshExprOperator *
CwshExprStack::
getLastOperator()
{
  setLastOperator();

  return last_opr_;
}

void
CwshExprStack::
dumpStack()
{
  std::cerr << "Stack: ";

  CwshExprStackNodeList::iterator pstack_node1 = stack_nodes_.begin();
  CwshExprStackNodeList::iterator pstack_node2 = stack_nodes_.end  ();

  for ( ; pstack_node1 != pstack_node2; ++pstack_node1) {
    (*pstack_node1)->dump();

    std::cerr << " ";
  }

  std::cerr << std::endl;

  std::cerr << "Current: ";

  if (pstack_node_ != stack_nodes_.end())
    (*pstack_node_)->dump();
  else
    std::cerr << "End";

  std::cerr << std::endl;
}

CwshExprStackNode::
CwshExprStackNode(const string &value) :
 type_(CWSH_STACK_NODE_TYPE_VALUE), opr_(NULL), value_(value)
{
}

CwshExprStackNode::
CwshExprStackNode(CwshExprOperator *opr) :
 type_(CWSH_STACK_NODE_TYPE_OPERATOR), opr_(opr), value_("")
{
}

CwshExprStackNode::
~CwshExprStackNode()
{
}

void
CwshExprStackNode::
print() const
{
  if (type_ == CWSH_STACK_NODE_TYPE_VALUE)
    std::cout << "Value " << value_ << std::endl;
  else
    std::cout << "Operator " << opr_->getToken() << std::endl;
}

void
CwshExprStackNode::
dump() const
{
  if (type_ == CWSH_STACK_NODE_TYPE_VALUE)
    std::cerr << "<value>" << value_;
  else
    std::cerr << "<opr>" << opr_->getToken();
}
