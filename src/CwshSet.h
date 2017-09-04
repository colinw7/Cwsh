#ifndef CWSH_SET_H
#define CWSH_SET_H

enum class CwshSetAssignType {
  NONE,
  EQUALS,
  PLUS_EQUALS,
  MINUS_EQUALS,
  TIMES_EQUALS,
  DIVIDE_EQUALS,
  MODULUS_EQUALS,
  AND_EQUALS,
  OR_EQUALS,
  XOR_EQUALS,
  INCREMENT,
  DECREMENT
};

class CwshSet {
 public:
  CwshSet(Cwsh *cwsh);

  void parseSet(const std::string &str, std::string &name, int *index,
                CwshVariableType *type, std::vector<std::string> &values);
  void processSet(const std::string &name, int index, CwshVariableType type,
                  std::vector<std::string> &values);

  void parseAssign(const std::string &str, std::string &name, int *index,
                   CwshSetAssignType *assign_type, std::string &expr);
  void processAssign(const std::string &name, int index,
                     CwshSetAssignType assign_type, const std::string &expr);

 private:
  void              parseVariable(const std::string &str, uint *i, std::string &name, int *index);
  CwshSetAssignType parseAssignType(const std::string &str, uint *i);
  void              setValues(const std::string &str, uint *i, CwshVariableType *type,
                              std::vector<std::string> &values);

 private:
  CPtr<Cwsh> cwsh_;
};

#endif
