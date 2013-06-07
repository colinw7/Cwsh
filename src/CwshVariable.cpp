#include "CwshI.h"

enum CwshGetVariableType {
  CWSH_GET_VARIABLE_VALUES,
  CWSH_GET_VARIABLE_SIZE,
  CWSH_GET_VARIABLE_EXISTS
};

enum CwshVariableModifierType {
  CWSH_VARIABLE_MODIFIER_NONE,
  CWSH_VARIABLE_MODIFIER_ROOT,
  CWSH_VARIABLE_MODIFIER_EXTENSION,
  CWSH_VARIABLE_MODIFIER_HEADER,
  CWSH_VARIABLE_MODIFIER_TAIL,
  CWSH_VARIABLE_MODIFIER_QUOTE_WORDLIST,
  CWSH_VARIABLE_MODIFIER_QUOTE_PATTERN
};

string
CwshVariableMgr::
lower_env_names_[] = {
  "home",
  "path",
  "shell",
  "term",
  "user"
};

string
CwshVariableMgr::
upper_env_names_[] = {
  "HOME",
  "PATH",
  "SHELL",
  "TERM",
  "USER"
};

CwshVariableMgr::
CwshVariableMgr(Cwsh *cwsh) :
 cwsh_(cwsh)
{
}

CwshVariableMgr::
CwshVariableMgr(const CwshVariableMgr &mgr) :
 cwsh_(mgr.cwsh_)
{
  CwshVariableList::const_iterator pvariable1 = mgr.variables_.begin();
  CwshVariableList::const_iterator pvariable2 = mgr.variables_.end  ();

  for ( ; pvariable1 != pvariable2; ++pvariable1)
    variables_.push_back(new CwshVariable(**pvariable1));

  CwshVariableMgrArray::const_iterator pstack1 = mgr.stack_.begin();
  CwshVariableMgrArray::const_iterator pstack2 = mgr.stack_.end  ();

  for ( ; pstack1 != pstack2; ++pstack1)
    stack_.push_back(new CwshVariableMgr(**pstack1));
}

CwshVariableMgr::
~CwshVariableMgr()
{
  clear();
}

void
CwshVariableMgr::
define(const CwshVariableName &name)
{
  if (cwsh_->getDebug())
    cout << "define " << name << endl;

  undefine(name);

  CwshVariable *variable = new CwshVariable(cwsh_, name, "");

  variables_.push_back(variable);

  variables_.sort(CwshVariablesCmp());
}

void
CwshVariableMgr::
define(const CwshVariableName &name, const CwshVariableValue &value)
{
  if (cwsh_->getDebug())
    cout << "define " << name << "='" << value << "'" << endl;

  undefine(name);

  CwshVariable *variable = new CwshVariable(cwsh_, name, value);

  variables_.push_back(variable);

  variables_.sort(CwshVariablesCmp());
}

void
CwshVariableMgr::
define(const CwshVariableName &name, int value)
{
  if (cwsh_->getDebug())
    cout << "define " << name << "='" << value << "'" << endl;

  undefine(name);

  if (name == "_debug")
    cwsh_->setDebug(value != 0);

  CwshVariable *variable = new CwshVariable(cwsh_, name, CStrUtil::toString(value));

  variables_.push_back(variable);

  variables_.sort(CwshVariablesCmp());
}

void
CwshVariableMgr::
define(const CwshVariableName &name, const CwshVariableValueArray &values)
{
  if (cwsh_->getDebug()) {
    string value = CStrUtil::toString(values, " ");

    cout << "define " << name << "='" << value << "'" << endl;
  }

  undefine(name);

  CwshVariable *variable = new CwshVariable(cwsh_, name, values);

  variables_.push_back(variable);

  variables_.sort(CwshVariablesCmp());
}

void
CwshVariableMgr::
define(const CwshVariableName &name, const char **values, int num_values)
{
  if (cwsh_->getDebug()) {
    string value = CStrUtil::toString(values, num_values, " ");

    cout << "define " << name << "='" << value << "'" << endl;
  }

  undefine(name);

  CwshVariable *variable = new CwshVariable(cwsh_, name, values, num_values);

  variables_.push_back(variable);

  variables_.sort(CwshVariablesCmp());
}

void
CwshVariableMgr::
undefine(const CwshVariableName &name)
{
  CwshVariable *variable = lookup(name);

  if (variable != NULL) {
    variables_.remove(variable);

    delete variable;
  }
}

CwshVariable *
CwshVariableMgr::
lookup(const CwshVariableName &name) const
{
  CwshVariableList::const_iterator pvariable1 = variables_.begin();
  CwshVariableList::const_iterator pvariable2 = variables_.end  ();

  for ( ; pvariable1 != pvariable2; ++pvariable1)
    if ((*pvariable1)->getName() == name)
      return *pvariable1;

  return NULL;
}

void
CwshVariableMgr::
listVariables() const
{
  CwshVariableList::const_iterator pvariable1 = variables_.begin();
  CwshVariableList::const_iterator pvariable2 = variables_.end  ();

  for ( ; pvariable1 != pvariable2; ++pvariable1)
    (*pvariable1)->print();
}

void
CwshVariableMgr::
clear()
{
  for_each(variables_.begin(), variables_.end(), CDeletePointer());

  variables_.clear();

  for_each(stack_.begin(), stack_.end(), CDeletePointer());

  stack_.clear();
}

void
CwshVariableMgr::
save()
{
  CwshVariableMgr *copy = new CwshVariableMgr(*this);

  stack_.push_back(copy);
}

void
CwshVariableMgr::
restore()
{
  if (stack_.size() == 0)
    CWSH_THROW("Not in save state.");

  CwshVariableMgr *variable_mgr = stack_[stack_.size() - 1];

  stack_.pop_back();

  clear();

  copy(variable_mgr->variables_.begin(), variable_mgr->variables_.end(),
       back_inserter(variables_));

  variable_mgr->variables_.clear();

  copy(variable_mgr->stack_.begin(), variable_mgr->stack_.end(),
       back_inserter(stack_));

  variable_mgr->stack_.clear();

  delete variable_mgr;
}

bool
CwshVariableMgr::
isEnvironmentVariableLower(const string &name)
{
  int num_env_names_ = sizeof(lower_env_names_)/sizeof(string);

  for (int i = 0; i < num_env_names_; i++)
    if (lower_env_names_[i] == name)
      return true;

  return false;
}

bool
CwshVariableMgr::
isEnvironmentVariableUpper(const string &name)
{
  int num_env_names_ = sizeof(upper_env_names_)/sizeof(string);

  for (int i = 0; i < num_env_names_; i++)
    if (upper_env_names_[i] == name)
      return true;

  return false;
}

void
CwshVariableMgr::
updateEnvironmentVariable(CwshVariable *variable)
{
  string name = CStrUtil::toUpper(variable->getName());

  string value;

  int num_values = variable->getNumValues();

  for (int i = 0; i < num_values; i++) {
    if (i > 0)
      value += ":";

    value += variable->getValue(i);
  }

  COSEnv::setenv(name, value);
}

CwshVariable::
CwshVariable(Cwsh *cwsh, const CwshVariableName &name, const CwshVariableValue &value) :
 cwsh_(cwsh), name_(name)
{
  checkName();

  values_.push_back(value);

  init();
}

CwshVariable::
CwshVariable(Cwsh *cwsh, const CwshVariableName &name, int value) :
 cwsh_(cwsh), name_(name)
{
  checkName();

  CwshVariableValue value1 = CStrUtil::toString(value);

  values_.push_back(value1);

  init();
}

CwshVariable::
CwshVariable(Cwsh *cwsh, const CwshVariableName &name, const CwshVariableValueArray &values) :
 cwsh_(cwsh), name_(name), values_(values)
{
  checkName();

  init();
}

CwshVariable::
CwshVariable(Cwsh *cwsh, const CwshVariableName &name, const char **values, int num_values) :
 cwsh_(cwsh), name_(name)
{
  checkName();

  for (int i = 0; i < num_values; ++i)
    values_.push_back(values[i]);

  init();
}

CwshVariable::
CwshVariable(const CwshVariable &variable) :
 cwsh_(variable.cwsh_), name_(variable.name_), type_(variable.type_), env_var_(variable.env_var_)
{
  int num_values = variable.values_.size();

  for (int i = 0; i < num_values; ++i)
    values_.push_back(variable.values_[i]);
}

CwshVariable::
~CwshVariable()
{
}

void
CwshVariable::
checkName()
{
  int len = name_.size();

  if (len < 1)
    CWSH_THROW("NULL variable Name");

  if (name_[0] != '_' && ! isalpha(name_[0]))
    CWSH_THROW("Variable name must begin with a letter or underscore");

  for (int i = 1; i < len; i++)
    if (name_[i] != '_' && ! isalnum(name_[i]))
      CWSH_THROW("Variable name must contain only letters, "
                 "numbers or underscore");
}

void
CwshVariable::
init()
{
  env_var_ = cwsh_->isEnvironmentVariableLower(name_);

  if (env_var_)
    cwsh_->updateEnvironmentVariable(this);
}

const CwshVariableName &
CwshVariable::
getName() const
{
  return name_;
}

CwshVariableType
CwshVariable::
getType() const
{
  return type_;
}

int
CwshVariable::
getNumValues() const
{
  return values_.size();
}

const CwshVariableValueArray &
CwshVariable::
getValues() const
{
  return values_;
}

const CwshVariableValue &
CwshVariable::
getValue(int pos) const
{
  return values_[pos];
}

void
CwshVariable::
setValue(int pos, const CwshVariableValue &value)
{
  values_[pos] = value;
}

void
CwshVariable::
shift()
{
  int len = values_.size();

  for (int i = 1; i < len; i++)
    values_[i - 1] = values_[i];

  values_.pop_back();
}

void
CwshVariable::
print() const
{
  cout << name_ << " ";

  int num_values = values_.size();

  if (num_values > 1)
    cout << '(';

  for (int i = 0; i < num_values; i++) {
    if (i > 0)
      cout << " ";

    cout << values_[i];
  }

  if (num_values > 1)
    cout << ')';

  cout << endl;
}

int
CwshVariablesCmp::
operator()(const CwshVariable *variable1, const CwshVariable *variable2)
{
  return (variable1->getName() < variable2->getName());
}
