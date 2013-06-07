#ifndef CWSH_EXPR_PROCESS_H
#define CWSH_EXPR_PROCESS_H

enum CwshExprProcessValueType {
  CWSH_EXPR_PROCESS_VALUE_TYPE_STRING,
  CWSH_EXPR_PROCESS_VALUE_TYPE_INTEGER,
};

class CwshExprProcess {
 public:
  CwshExprProcess();

  std::string process(CwshExprOperator *opr, const std::string &value);
  std::string process(const std::string &value1, CwshExprOperator *opr, const std::string &value2);

 private:
  CwshExprProcessValueType getValueType(const std::string &value, int *integer);
};

#endif
