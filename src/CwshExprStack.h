#ifndef CWSH_EXPR_STACK_H
#define CWSH_EXPR_STACK_H

namespace Cwsh {

enum class ExprStackNodeType {
  OPERATOR,
  VALUE,
};

class ExprStackStack {
 public:
  ExprStackStack();
 ~ExprStackStack();

  void push(const std::string &value);
  void push(ExprOperator *opr);

  bool pop(std::string &value);
  bool pop(ExprOperator **opr);

  void remove(ExprStackNode *stackNode);

  void restart(bool flag);

  ExprStack *start();
  void       end();

  void startTemporary();
  void endTemporary();

  ExprStack *getCurrent() const;

  //void setCurrent(ExprStack *stack);

  void toStart();
  void toEnd();

  void setInBrackets(bool flag);
  bool getInBrackets();

  void signalRestart();
  bool checkRestart();

  ExprStackNode *getCurrentNode();

  void toNext();
  void toPrev();

  void          setLastOperator();
  ExprOperator *getLastOperator();

 private:
  using ExprStackP = std::shared_ptr<ExprStack>;

  ExprStackP     current_;
  ExprStackArray stacks_;
};

//---

class ExprStack {
  CINST_COUNT_MEMBER(ExprStack);

 public:
  ExprStack();
 ~ExprStack();

  void push(const std::string &value);
  void push(ExprOperator *opr);

  bool pop(std::string &value);
  bool pop(ExprOperator **opr);

  void remove(ExprStackNode *stackNode);

  void restart(bool flag);

  void toStart();
  void toEnd();

  void setInBrackets(bool flag);
  bool getInBrackets();

  void signalRestart();
  bool checkRestart();

  ExprStackNode *getCurrent();

  void toNext();
  void toPrev();

  void          setLastOperator();
  ExprOperator *getLastOperator();

  void dumpStack();

 private:
  void push(ExprStackNode *stackNode);

 private:
  bool                        restart_   { false };
  bool                        inBracket_ { false };
  ExprOperator *              lastOpr_   { nullptr };
  ExprStackNodeList           stackNodes_;
  ExprStackNodeList::iterator pstackNode_;
  bool                        debug_     { false };
};

//---

class ExprStackNode {
  CINST_COUNT_MEMBER(ExprStackNode);

 public:
  ExprStackNode(const std::string &value);
  ExprStackNode(ExprOperator *opr);

 ~ExprStackNode();

  ExprStackNodeType getType() const { return type_; }

  ExprOperator *getOperator() const { return opr_; }
  std::string   getValue   () const { return value_; }

  void print() const;
  void dump() const;

 private:
  ExprStackNodeType type_;
  ExprOperator*     opr_;
  std::string       value_;
};

}

#endif
