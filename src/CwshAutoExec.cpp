#include <CwshI.h>

template<typename T>
class CwshAutoExecListValueDisplay {
 public:
  void operator()(const typename T::value_type &alias) {
    alias.second->display();
  }
};

CwshAutoExecMgr::
CwshAutoExecMgr(Cwsh *cwsh) :
 cwsh_(cwsh)
{
}

CwshAutoExecMgr::
~CwshAutoExecMgr()
{
}

void
CwshAutoExecMgr::
define(const CwshAutoExecName &suffix, const CwshAutoExecValue &value)
{
  CwshAutoExec *alias = new CwshAutoExec(suffix, value);

  auto_execs_.setValue(suffix, alias);
}

void
CwshAutoExecMgr::
undefine(const CwshAutoExecName &suffix)
{
  auto_execs_.unsetValue(suffix);
}

CwshAutoExec *
CwshAutoExecMgr::
lookup(const std::string &suffix) const
{
  return auto_execs_.getValue(suffix);
}

void
CwshAutoExecMgr::
display() const
{
  for (auto &auto_exec : auto_execs_)
    CwshAutoExecListValueDisplay<AutoExecList>()(auto_exec);
}

std::string
CwshAutoExecMgr::
getAutoExecsMsg() const
{
  std::string msg;

  for (const auto &auto_exec : auto_execs_) {
    if (! msg.empty())
      msg += "#";

    msg += auto_exec.second->getName () + "#" + auto_exec.second->getValue();
  }

  return msg;
}

//-------------------

CwshAutoExec::
CwshAutoExec(const std::string &suffix, const std::string &value) :
 suffix_(suffix), value_(value)
{
}

CwshAutoExec::
~CwshAutoExec()
{
}

void
CwshAutoExec::
display() const
{
  std::cout << suffix_ << " " << value_ << std::endl;
}

bool
CwshAutoExec::
substitute(const std::string &name, std::string &cmd, std::vector<std::string> &args)
{
  cmd = value_;

  args.push_back(name);

  return true;
}
