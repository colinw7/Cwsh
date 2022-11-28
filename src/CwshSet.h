#ifndef CWSH_SET_H
#define CWSH_SET_H

namespace Cwsh {

enum class SetAssignType {
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

class Set {
 public:
  Set(App *cwsh);

  void parseSet(const std::string &str, std::string &name, int *index,
                VariableType *type, std::vector<std::string> &values);
  void processSet(const std::string &name, int index, VariableType type,
                  std::vector<std::string> &values);

  void parseAssign(const std::string &str, std::string &name, int *index,
                   SetAssignType *assignType, std::string &expr);
  void processAssign(const std::string &name, int index,
                     SetAssignType assignType, const std::string &expr);

 private:
  void          parseVariable(const std::string &str, uint *i, std::string &name, int *index);
  SetAssignType parseAssignType(const std::string &str, uint *i);
  void          setValues(const std::string &str, uint *i, VariableType *type,
                          std::vector<std::string> &values);

 private:
  CPtr<App> cwsh_;
};

}

#endif
