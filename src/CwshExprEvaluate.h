#ifndef CWSH_EVAULATE_H
#define CWSH_EVAULATE_H

class CwshExprEvaluate {
 public:
  CwshExprEvaluate(Cwsh *cwsh, const std::string &expression);

  int process();

 private:
  std::string evaluate();
  std::string evaluateStack();
  std::string unstack();
  std::string backUnstack();
  void        unstackUnary();
  void        unstackBinary();
  void        unstackBracketed();
  void        skip();
  bool        checkUnstack(CwshExprOperator *opr);

 private:
  CPtr<Cwsh>                   cwsh_;
  std::string                  expression_;
  CAutoPtr<CwshExprStackStack> stack_;
};

#endif
