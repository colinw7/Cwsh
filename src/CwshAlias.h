#ifndef CWSH_ALIAS_H
#define CWSH_ALIAS_H

#include <CPtr.h>

namespace Cwsh {

//---

class AliasMgr {
  CINST_COUNT_MEMBER(AliasMgr);

 public:
  using AliasList = std::map<std::string, AliasP>;

 public:
  AliasMgr(App *cwsh);
 ~AliasMgr();

  Alias *define(const std::string &name, const std::string &value);

  void undefine(const std::string &name);

  Alias *lookup(const std::string &name) const;

  bool substitute(Cmd *cmd, CmdArray &cmds) const;

  void display(bool all) const;

  std::string getAliasesMsg() const;

 private:
  CPtr<App>   cwsh_;
  AliasList   aliases_;
  CPtr<Alias> lastAlias_;
};

//---

class Alias {
  CINST_COUNT_MEMBER(Alias);

 public:
  Alias(const std::string &name, const std::string &value);
 ~Alias();

  const std::string &getName() const { return name_; }

  const std::string &getValue() const { return value_; }
  void setValue(const std::string &value) { value_ = value; }

  const std::string &getFilename() const { return filename_; }
  void setFilename(const std::string &s) { filename_ = s; }

  int getLineNum() const { return lineNum_; }
  void setLineNum(int i) { lineNum_ = i; }

  void display(bool all) const;

  void displayValue(bool all) const;

 private:
  std::string name_;
  std::string value_;
  std::string filename_;
  int         lineNum_ { -1 };
};

//---

class AliasEq {
 public:
  AliasEq(const std::string &name) : name_(name) { }

  bool operator()(const Alias *alias) { return name_ == alias->getName(); }

 private:
  const std::string name_;
};

//---

}

#endif
