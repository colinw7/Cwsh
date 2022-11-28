#ifndef CWSH_EVAULATE_H
#define CWSH_EVAULATE_H

namespace Cwsh {

class ExprEvaluate {
 public:
  ExprEvaluate(App *cwsh, const std::string &expression);

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
  bool        checkUnstack(ExprOperator *opr);

 private:
  using ExprStackStackP = std::unique_ptr<ExprStackStack>;

  CPtr<App>       cwsh_;
  std::string     expression_;
  ExprStackStackP stack_;
};

}

#endif
