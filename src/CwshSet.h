#ifndef CWSH_SET_H
#define CWSH_SET_H

enum CwshSetAssignType {
  CWSH_SET_ASSIGN_TYPE_NONE,
  CWSH_SET_ASSIGN_TYPE_EQUALS,
  CWSH_SET_ASSIGN_TYPE_PLUS_EQUALS,
  CWSH_SET_ASSIGN_TYPE_MINUS_EQUALS,
  CWSH_SET_ASSIGN_TYPE_TIMES_EQUALS,
  CWSH_SET_ASSIGN_TYPE_DIVIDE_EQUALS,
  CWSH_SET_ASSIGN_TYPE_MODULUS_EQUALS,
  CWSH_SET_ASSIGN_TYPE_AND_EQUALS,
  CWSH_SET_ASSIGN_TYPE_OR_EQUALS,
  CWSH_SET_ASSIGN_TYPE_XOR_EQUALS,
  CWSH_SET_ASSIGN_TYPE_INCREMENT,
  CWSH_SET_ASSIGN_TYPE_DECREMENT,
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
