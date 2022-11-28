#ifndef CWSH_VARIABLE_H
#define CWSH_VARIABLE_H

namespace Cwsh {

class VariableMgr {
 public:
  VariableMgr(App *cwsh);
  VariableMgr(const VariableMgr &mgr);

 ~VariableMgr();

  Variable *define(const std::string &name);
  Variable *define(const std::string &name, const std::string &value);
  Variable *define(const std::string &name, int value);
  Variable *define(const std::string &name, const VariableValueArray &values);
  Variable *define(const std::string &name, const char **values, int numValues);

  void undefine(const std::string &name);

  Variable *lookup(const std::string &name) const;

  const auto &variables() const { return variables_; }

  void listVariables(bool all) const;

  void save();
  void restore();

  bool isEnvironmentVariableLower(const std::string &name);
  bool isEnvironmentVariableUpper(const std::string &name);

  void updateEnvironmentVariable(Variable *variable);

 private:
  void clear();

 private:
  CPtr<App>        cwsh_;
  VariableList     variables_;
  VariableMgrArray stack_;

  static std::string lowerEnvNames_[];
  static std::string upperEnvNames_[];
};

//---

enum class VariableType {
  NONE,
  WORD,
  WORDLIST
};

class Variable {
  CINST_COUNT_MEMBER(Variable);

 public:
  Variable(App *cwsh, const std::string &name, const std::string &value);
  Variable(App *cwsh, const std::string &name, int value);
  Variable(App *cwsh, const std::string &name, const VariableValueArray &values);
  Variable(App *cwsh, const std::string &name, const char **values, int numValues);
  Variable(const Variable &variable);
 ~Variable();

  const std::string &getName() const;
  VariableType       getType() const;

  uint                     getNumValues() const;
  const VariableValueArray &getValues() const;

  const std::string &getValue(int pos) const;
  void               setValue(int pos, const std::string &value);

  bool isEnvVar() const { return envVar_; }

  const std::string &getFilename() const { return filename_; }
  void setFilename(const std::string &v) { filename_ = v; }

  int getLineNum() const { return lineNum_; }
  void setLineNum(int i) { lineNum_ = i; }

  void shift();

  void print(bool all) const;

 private:
  void init();

  void checkName();

 private:
  CPtr<App>          cwsh_;
  std::string        name_;
  VariableType       type_ { VariableType::NONE };
  VariableValueArray values_;
  bool               envVar_ { false };
  std::string        filename_;
  int                lineNum_ { -1 };
};

//---

class VariablesCmp {
 public:
  int operator()(const Variable *variable1, const Variable *variable2);
};

}

#endif
