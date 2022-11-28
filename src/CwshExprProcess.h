#ifndef CWSH_EXPR_PROCESS_H
#define CWSH_EXPR_PROCESS_H

namespace Cwsh {

enum class ExprProcessValueType {
  STRING,
  INTEGER,
};

class ExprProcess {
 public:
  ExprProcess();

  std::string process(ExprOperator *opr, const std::string &value);
  std::string process(const std::string &value1, ExprOperator *opr, const std::string &value2);

 private:
  ExprProcessValueType getValueType(const std::string &value, int *integer);
};

}

#endif
