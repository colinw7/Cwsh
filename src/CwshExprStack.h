#ifndef CWSH_EXPR_STACK_H
#define CWSH_EXPR_STACK_H

enum CwshExprStackNodeType {
  CWSH_STACK_NODE_TYPE_OPERATOR,
  CWSH_STACK_NODE_TYPE_VALUE,
};

class CwshExprStackStack {
 public:
  CwshExprStackStack();
 ~CwshExprStackStack();

  void push(const std::string &value);
  void push(CwshExprOperator *opr);

  bool pop(std::string &value);
  bool pop(CwshExprOperator **opr);

  void remove(CwshExprStackNode *stack_node);

  void restart(bool flag);

  CwshExprStack *start();
  void           end();

  void startTemporary();
  void endTemporary();

  CwshExprStack *getCurrent() const;

  void setCurrent(CwshExprStack *stack);

  void toStart();
  void toEnd();

  void setInBrackets(bool flag);
  bool getInBrackets();

  void signalRestart();
  bool checkRestart();

  CwshExprStackNode *getCurrentNode();

  void toNext();
  void toPrev();

  void              setLastOperator();
  CwshExprOperator *getLastOperator();

 private:
  CAutoPtr<CwshExprStack> current_;
  CwshExprStackArray      stacks_;
};

class CwshExprStack {
  CINST_COUNT_MEMBER(CwshExprStack);

 public:
  CwshExprStack();
 ~CwshExprStack();

  void push(const std::string &value);
  void push(CwshExprOperator *opr);

  bool pop(std::string &value);
  bool pop(CwshExprOperator **opr);

  void remove(CwshExprStackNode *stack_node);

  void restart(bool flag);

  void toStart();
  void toEnd();

  void setInBrackets(bool flag);
  bool getInBrackets();

  void signalRestart();
  bool checkRestart();

  CwshExprStackNode *getCurrent();

  void toNext();
  void toPrev();

  void              setLastOperator();
  CwshExprOperator *getLastOperator();

  void dumpStack();

 private:
  void push(CwshExprStackNode *stack_node);

 private:
  bool                             restart_;
  bool                             in_bracket_;
  CwshExprOperator                *last_opr_;
  CwshExprStackNodeList            stack_nodes_;
  CwshExprStackNodeList::iterator  pstack_node_;
  bool                             debug_;
};

class CwshExprStackNode {
  CINST_COUNT_MEMBER(CwshExprStackNode);

 public:
  CwshExprStackNode(const std::string &value);
  CwshExprStackNode(CwshExprOperator *opr);

 ~CwshExprStackNode();

  CwshExprStackNodeType getType() const { return type_; }

  CwshExprOperator *getOperator() const { return opr_; }
  std::string       getValue   () const { return value_; }

  void print() const;
  void dump() const;

 private:
  CwshExprStackNodeType  type_;
  CwshExprOperator      *opr_;
  std::string            value_;
};

#endif
