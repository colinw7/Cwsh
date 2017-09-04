#ifndef CWSH_VARIABLE_H
#define CWSH_VARIABLE_H

class CwshVariableMgr {
 public:
  CwshVariableMgr(Cwsh *cwsh);
  CwshVariableMgr(const CwshVariableMgr &mgr);

 ~CwshVariableMgr();

  CwshVariable *define(const CwshVariableName &name);
  CwshVariable *define(const CwshVariableName &name, const CwshVariableValue &value);
  CwshVariable *define(const CwshVariableName &name, int value);
  CwshVariable *define(const CwshVariableName &name, const CwshVariableValueArray &values);
  CwshVariable *define(const CwshVariableName &name, const char **values, int num_values);

  void undefine(const CwshVariableName &name);

  CwshVariable *lookup(const CwshVariableName &name) const;

  CwshVariableList::iterator variablesBegin() {
    return variables_.begin();
  }

  CwshVariableList::iterator variablesEnd() {
    return variables_.end();
  }

  void listVariables(bool all) const;

  void save();
  void restore();

  bool isEnvironmentVariableLower(const std::string &name);
  bool isEnvironmentVariableUpper(const std::string &name);

  void updateEnvironmentVariable(CwshVariable *variable);

 private:
  void clear();

 private:
  CPtr<Cwsh>           cwsh_;
  CwshVariableList     variables_;
  CwshVariableMgrArray stack_;

  static std::string lower_env_names_[];
  static std::string upper_env_names_[];
};

//---

enum class CwshVariableType {
  WORD,
  WORDLIST
};

class CwshVariable {
  CINST_COUNT_MEMBER(CwshVariable);

 public:
  CwshVariable(Cwsh *cwsh, const CwshVariableName &name, const CwshVariableValue &value);
  CwshVariable(Cwsh *cwsh, const CwshVariableName &name, int value);
  CwshVariable(Cwsh *cwsh, const CwshVariableName &name, const CwshVariableValueArray &values);
  CwshVariable(Cwsh *cwsh, const CwshVariableName &name, const char **values, int num_values);
  CwshVariable(const CwshVariable &variable);
 ~CwshVariable();

  const CwshVariableName &getName() const;
  CwshVariableType        getType() const;

  int                           getNumValues() const;
  const CwshVariableValueArray &getValues() const;

  const CwshVariableValue &getValue(int pos) const;
  void                     setValue(int pos, const CwshVariableValue &value);

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
  CPtr<Cwsh>             cwsh_;
  CwshVariableName       name_;
  CwshVariableType       type_;
  CwshVariableValueArray values_;
  bool                   envVar_ { false };
  std::string            filename_;
  int                    lineNum_ { -1 };
};

//---

class CwshVariablesCmp {
 public:
  int operator()(const CwshVariable *variable1, const CwshVariable *variable2);
};

#endif
