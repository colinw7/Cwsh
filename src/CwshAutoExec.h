#ifndef CWSH_AUTO_EXEC_H
#define CWSH_AUTO_EXEC_H

#include <CPtr.h>

class CwshAutoExecMgr {
  CINST_COUNT_MEMBER(CwshAutoExecMgr);

 public:
  typedef CAutoPtrMap<std::string,CwshAutoExec> AutoExecList;

 public:
  CwshAutoExecMgr(Cwsh *cwsh);
 ~CwshAutoExecMgr();

  void define(const CwshAutoExecName &suffix, const CwshAutoExecValue &value);

  void undefine(const CwshAutoExecName &suffix);

  CwshAutoExec *lookup(const CwshAutoExecName &suffix) const;

  void display() const;

  std::string getAutoExecsMsg() const;

 private:
  CPtr<Cwsh>   cwsh_;
  AutoExecList auto_execs_;
};

//---

class CwshAutoExec {
  CINST_COUNT_MEMBER(CwshAutoExec);

 public:
  CwshAutoExec(const CwshAutoExecName &suffix, const CwshAutoExecValue &value);
 ~CwshAutoExec();

  const CwshAutoExecName  &getName () const { return suffix_ ; }
  const CwshAutoExecValue &getValue() const { return value_; }

  void setValue(const CwshAutoExecValue &value) { value_ = value; }

  void display() const;

  bool substitute(const std::string &suffix, std::string &cmd, std::vector<std::string> &args);

 private:
  CwshAutoExecName  suffix_;
  CwshAutoExecValue value_;
};

//---

class CwshAutoExecEq {
 public:
  CwshAutoExecEq(const std::string &suffix) : suffix_(suffix) { }

  bool operator()(const CwshAutoExec *auto_exec) { return suffix_ == auto_exec->getName(); }

 private:
  const std::string suffix_;
};

#endif
