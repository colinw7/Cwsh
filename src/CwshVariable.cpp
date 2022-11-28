#include <CwshI.h>

namespace Cwsh {

enum class GetVariableType {
  CWSH_GET_VARIABLE_VALUES,
  CWSH_GET_VARIABLE_SIZE,
  CWSH_GET_VARIABLE_EXISTS
};

enum class VariableModifierType {
  CWSH_VARIABLE_MODIFIER_NONE,
  CWSH_VARIABLE_MODIFIER_ROOT,
  CWSH_VARIABLE_MODIFIER_EXTENSION,
  CWSH_VARIABLE_MODIFIER_HEADER,
  CWSH_VARIABLE_MODIFIER_TAIL,
  CWSH_VARIABLE_MODIFIER_QUOTE_WORDLIST,
  CWSH_VARIABLE_MODIFIER_QUOTE_PATTERN
};

std::string
VariableMgr::
lowerEnvNames_[] = {
  "home",
  "path",
  "shell",
  "term",
  "user"
};

std::string
VariableMgr::
upperEnvNames_[] = {
  "HOME",
  "PATH",
  "SHELL",
  "TERM",
  "USER"
};

VariableMgr::
VariableMgr(App *cwsh) :
 cwsh_(cwsh)
{
}

VariableMgr::
VariableMgr(const VariableMgr &mgr) :
 cwsh_(mgr.cwsh_)
{
  for (auto &variable : mgr.variables_)
    variables_.push_back(new Variable(*variable));

  for (auto &stack : mgr.stack_)
    stack_.push_back(new VariableMgr(*stack));
}

VariableMgr::
~VariableMgr()
{
  clear();
}

Variable *
VariableMgr::
define(const std::string &name)
{
  if (cwsh_->getDebug())
    std::cout << "define " << name << "\n";

  undefine(name);

  auto *variable = new Variable(cwsh_, name, "");

  variables_.push_back(variable);

  variables_.sort(VariablesCmp());

  return variable;
}

Variable *
VariableMgr::
define(const std::string &name, const std::string &value)
{
  if (cwsh_->getDebug())
    std::cout << "define " << name << "='" << value << "'\n";

  undefine(name);

  auto *variable = new Variable(cwsh_, name, value);

  variables_.push_back(variable);

  variables_.sort(VariablesCmp());

  return variable;
}

Variable *
VariableMgr::
define(const std::string &name, int value)
{
  if (cwsh_->getDebug())
    std::cout << "define " << name << "='" << value << "'\n";

  undefine(name);

  if (name == "_debug")
    cwsh_->setDebug(value != 0);

  auto *variable = new Variable(cwsh_, name, CStrUtil::toString(value));

  variables_.push_back(variable);

  variables_.sort(VariablesCmp());

  return variable;
}

Variable *
VariableMgr::
define(const std::string &name, const VariableValueArray &values)
{
  if (cwsh_->getDebug()) {
    std::string value = CStrUtil::toString(values, " ");

    std::cout << "define " << name << "='" << value << "'\n";
  }

  undefine(name);

  auto *variable = new Variable(cwsh_, name, values);

  variables_.push_back(variable);

  variables_.sort(VariablesCmp());

  return variable;
}

Variable *
VariableMgr::
define(const std::string &name, const char **values, int numValues)
{
  if (cwsh_->getDebug()) {
    std::string value = CStrUtil::toString(values, numValues, " ");

    std::cout << "define " << name << "='" << value << "'\n";
  }

  undefine(name);

  auto *variable = new Variable(cwsh_, name, values, numValues);

  variables_.push_back(variable);

  variables_.sort(VariablesCmp());

  return variable;
}

void
VariableMgr::
undefine(const std::string &name)
{
  auto *variable = lookup(name);

  if (variable) {
    variables_.remove(variable);

    delete variable;
  }
}

Variable *
VariableMgr::
lookup(const std::string &name) const
{
  for (auto &variable : variables_) {
    if (variable->getName() == name)
      return variable;
  }

  return nullptr;
}

void
VariableMgr::
listVariables(bool all) const
{
  for (auto &variable : variables_)
    variable->print(all);
}

void
VariableMgr::
clear()
{
  for (auto &variable : variables_)
    delete variable;

  variables_.clear();

  for (auto &node : stack_)
    delete node;

  stack_.clear();
}

void
VariableMgr::
save()
{
  auto *copy = new VariableMgr(*this);

  stack_.push_back(copy);
}

void
VariableMgr::
restore()
{
  if (stack_.size() == 0)
    CWSH_THROW("Not in save state.");

  auto *variableMgr = stack_[stack_.size() - 1];

  stack_.pop_back();

  clear();

  copy(variableMgr->variables_.begin(), variableMgr->variables_.end(), back_inserter(variables_));

  variableMgr->variables_.clear();

  copy(variableMgr->stack_.begin(), variableMgr->stack_.end(), back_inserter(stack_));

  variableMgr->stack_.clear();

  delete variableMgr;
}

bool
VariableMgr::
isEnvironmentVariableLower(const std::string &name)
{
  int numEnvNames_ = sizeof(lowerEnvNames_)/sizeof(std::string);

  for (int i = 0; i < numEnvNames_; i++)
    if (lowerEnvNames_[i] == name)
      return true;

  return false;
}

bool
VariableMgr::
isEnvironmentVariableUpper(const std::string &name)
{
  int numEnvNames_ = sizeof(upperEnvNames_)/sizeof(std::string);

  for (int i = 0; i < numEnvNames_; i++)
    if (upperEnvNames_[i] == name)
      return true;

  return false;
}

void
VariableMgr::
updateEnvironmentVariable(Variable *variable)
{
  std::string name = CStrUtil::toUpper(variable->getName());

  std::string value;

  auto numValues = variable->getNumValues();

  for (uint i = 0; i < numValues; i++) {
    if (i > 0)
      value += ":";

    value += variable->getValue(i);
  }

  CEnvInst.set(name, value);
}

Variable::
Variable(App *cwsh, const std::string &name, const std::string &value) :
 cwsh_(cwsh), name_(name)
{
  checkName();

  values_.push_back(value);

  init();
}

Variable::
Variable(App *cwsh, const std::string &name, int value) :
 cwsh_(cwsh), name_(name)
{
  checkName();

  auto value1 = CStrUtil::toString(value);

  values_.push_back(value1);

  init();
}

Variable::
Variable(App *cwsh, const std::string &name, const VariableValueArray &values) :
 cwsh_(cwsh), name_(name), values_(values)
{
  checkName();

  init();
}

Variable::
Variable(App *cwsh, const std::string &name, const char **values, int numValues) :
 cwsh_(cwsh), name_(name)
{
  checkName();

  for (int i = 0; i < numValues; ++i)
    values_.push_back(values[i]);

  init();
}

Variable::
Variable(const Variable &variable) :
 cwsh_(variable.cwsh_), name_(variable.name_), type_(variable.type_), envVar_(variable.envVar_)
{
  int numValues = int(variable.values_.size());

  for (int i = 0; i < numValues; ++i)
    values_.push_back(variable.values_[i]);
}

Variable::
~Variable()
{
}

void
Variable::
checkName()
{
  int len = int(name_.size());

  if (len < 1)
    CWSH_THROW("NULL variable Name");

  if (name_[0] != '_' && ! isalpha(name_[0]))
    CWSH_THROW("Variable name must begin with a letter or underscore");

  for (int i = 1; i < len; i++)
    if (name_[i] != '_' && ! isalnum(name_[i]))
      CWSH_THROW("Variable name must contain only letters, numbers or underscore");
}

void
Variable::
init()
{
  envVar_ = cwsh_->isEnvironmentVariableLower(name_);

  if (envVar_)
    cwsh_->updateEnvironmentVariable(this);
}

const std::string &
Variable::
getName() const
{
  return name_;
}

VariableType
Variable::
getType() const
{
  return type_;
}

uint
Variable::
getNumValues() const
{
  return uint(values_.size());
}

const VariableValueArray &
Variable::
getValues() const
{
  return values_;
}

const std::string &
Variable::
getValue(int pos) const
{
  return values_[pos];
}

void
Variable::
setValue(int pos, const std::string &value)
{
  values_[pos] = value;
}

void
Variable::
shift()
{
  int len = int(values_.size());

  for (int i = 1; i < len; i++)
    values_[i - 1] = values_[i];

  values_.pop_back();
}

void
Variable::
print(bool all) const
{
  auto *mgr = CwshMgrInst;

  std::cout << mgr->varNameColorStr() << name_ << mgr->resetColorStr() << " ";

  std:: cout << mgr->varValueColorStr();

  int numValues = int(values_.size());

  if (numValues > 1)
    std::cout << '(';

  for (int i = 0; i < numValues; i++) {
    if (i > 0)
      std::cout << " ";

    std::cout << values_[i];
  }

  if (numValues > 1)
    std::cout << ')';

  std::cout << mgr->resetColorStr();

  if (all && getLineNum() > 0) {
    std::cout << " [";

    std::cout << mgr->locationColorStr() << getFilename() << ":" << getLineNum() <<
                 mgr->resetColorStr();

    std::cout << "]";
  }

  std::cout << "\n";
}

int
VariablesCmp::
operator()(const Variable *variable1, const Variable *variable2)
{
  return (variable1->getName() < variable2->getName());
}

}
