#include <CwshI.h>

namespace Cwsh {

//---

AutoExecMgr::
AutoExecMgr(App *cwsh) :
 cwsh_(cwsh)
{
}

AutoExecMgr::
~AutoExecMgr()
{
}

void
AutoExecMgr::
define(const std::string &suffix, const std::string &value)
{
  auto alias = std::make_shared<AutoExec>(suffix, value);

  autoExecs_[suffix] = alias;
}

void
AutoExecMgr::
undefine(const std::string &suffix)
{
  autoExecs_.erase(suffix);
}

AutoExec *
AutoExecMgr::
lookup(const std::string &suffix) const
{
  auto p = autoExecs_.find(suffix);

  return (p != autoExecs_.end() ? (*p).second.get() : nullptr);
}

void
AutoExecMgr::
display() const
{
  for (auto &autoExec : autoExecs_)
    autoExec.second->display();
}

std::string
AutoExecMgr::
getAutoExecsMsg() const
{
  std::string msg;

  for (const auto &autoExec : autoExecs_) {
    if (! msg.empty())
      msg += "#";

    msg += autoExec.second->getName () + "#" + autoExec.second->getValue();
  }

  return msg;
}

//-------------------

AutoExec::
AutoExec(const std::string &suffix, const std::string &value) :
 suffix_(suffix), value_(value)
{
}

AutoExec::
~AutoExec()
{
}

void
AutoExec::
display() const
{
  std::cout << suffix_ << " " << value_ << "\n";
}

bool
AutoExec::
substitute(const std::string &name, std::string &cmd, std::vector<std::string> &args)
{
  cmd = value_;

  args.push_back(name);

  return true;
}

//---

}
