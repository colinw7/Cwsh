#ifndef CWSH_AUTO_EXEC_H
#define CWSH_AUTO_EXEC_H

#include <CPtr.h>

namespace Cwsh {

//---

class AutoExecMgr {
  CINST_COUNT_MEMBER(AutoExecMgr);

 public:
  using AutoExecList = std::map<std::string, AutoExecP>;

 public:
  AutoExecMgr(App *cwsh);
 ~AutoExecMgr();

  void define(const std::string &suffix, const std::string &value);

  void undefine(const std::string &suffix);

  AutoExec *lookup(const std::string &suffix) const;

  void display() const;

  std::string getAutoExecsMsg() const;

 private:
  CPtr<App>    cwsh_;
  AutoExecList autoExecs_;
};

//---

class AutoExec {
  CINST_COUNT_MEMBER(AutoExec);

 public:
  AutoExec(const std::string &suffix, const std::string &value);
 ~AutoExec();

  const std::string &getName () const { return suffix_ ; }
  const std::string &getValue() const { return value_; }

  void setValue(const std::string &value) { value_ = value; }

  void display() const;

  bool substitute(const std::string &suffix, std::string &cmd, std::vector<std::string> &args);

 private:
  std::string suffix_;
  std::string value_;
};

//---

class AutoExecEq {
 public:
  AutoExecEq(const std::string &suffix) : suffix_(suffix) { }

  bool operator()(const AutoExec *autoExec) { return suffix_ == autoExec->getName(); }

 private:
  const std::string suffix_;
};

//---

}

#endif
