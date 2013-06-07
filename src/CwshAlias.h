#ifndef CWSH_ALIAS_H
#define CWSH_ALIAS_H

#include <CPtr.h>

class CwshAliasMgr {
  CINST_COUNT_MEMBER(CwshAliasMgr);

 public:
  typedef CAutoPtrMap<std::string,CwshAlias> AliasList;

 public:
  CwshAliasMgr(Cwsh *cwsh);
 ~CwshAliasMgr();

  void define(const CwshAliasName &name, const CwshAliasValue &value);

  void undefine(const CwshAliasName &name);

  CwshAlias *lookup(const CwshAliasName &name) const;

  bool substitute(CwshCmd *cmd, CwshCmdArray &cmds) const;

  void display() const;

  std::string getAliasesMsg() const;

 private:
  CPtr<Cwsh>      cwsh_;
  AliasList       aliases_;
  CPtr<CwshAlias> last_alias_;
};

class CwshAlias {
  CINST_COUNT_MEMBER(CwshAlias);

 public:
  CwshAlias(const CwshAliasName &name, const CwshAliasValue &value);
 ~CwshAlias();

  const CwshAliasName  &getName () const { return name_ ; }
  const CwshAliasValue &getValue() const { return value_; }

  void setValue(const CwshAliasValue &value) { value_ = value; }

  void display() const;

 private:
  CwshAliasName  name_;
  CwshAliasValue value_;
};

class CwshAliasEq {
 public:
  CwshAliasEq(const std::string &name) : name_(name) { }

  bool operator()(const CwshAlias *alias) { return name_ == alias->getName(); }

 private:
  const std::string name_;
};

#endif
