#include "CwshI.h"

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
lookup(const string &suffix) const
{
  return auto_execs_.getValue(suffix);
}

void
CwshAutoExecMgr::
display() const
{
  std::for_each(auto_execs_.begin(), auto_execs_.end(),
                CwshAutoExecListValueDisplay<AutoExecList>());
}

string
CwshAutoExecMgr::
getAutoExecsMsg() const
{
  string msg;

  AutoExecList::const_iterator palias1 = auto_execs_.begin();
  AutoExecList::const_iterator palias2 = auto_execs_.end  ();

  for ( ; palias1 != palias2; ++palias1) {
    if (! msg.empty()) msg += "#";

    msg += (*palias1).second->getName () + "#" + (*palias1).second->getValue();
  }

  return msg;
}

//-------------------

CwshAutoExec::
CwshAutoExec(const string &suffix, const string &value) :
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
  cout << suffix_ << " " << value_ << endl;
}

bool
CwshAutoExec::
substitute(const string &name, string &cmd, vector<string> &args)
{
  cmd = value_;

  args.push_back(name);

  return true;
}
