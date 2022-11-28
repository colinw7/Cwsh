#include <CwshI.h>

namespace Cwsh {

FunctionMgr::
FunctionMgr(App *cwsh) :
 cwsh_(cwsh)
{
}

Function *
FunctionMgr::
define(const std::string &name, const LineArray &lines)
{
  auto function = std::make_shared<Function>(cwsh_, name, lines);

  functionList_[name] = function;

  return function.get();
}

void
FunctionMgr::
undefine(const std::string &name)
{
  functionList_.erase(name);
}

Function *
FunctionMgr::
lookup(const std::string &name)
{
  auto p = functionList_.find(name);

  return (p != functionList_.end() ? (*p).second.get() : nullptr);
}

void
FunctionMgr::
listAll(bool all)
{
  for (const auto &pfunction : functionList_) {
    const auto &function = pfunction.second;

    function->list(all);
  }
}

//------

Function::
Function(App *cwsh, const std::string &name, const LineArray &lines) :
 cwsh_(cwsh), name_(name), lines_(lines)
{
}

Function::
~Function()
{
}

void
Function::
runProc(const ArgArray &args, CCommand::CallbackData data)
{
  auto *function = reinterpret_cast<Function *>(data);

  function->run(args);
}

void
Function::
run(const ArgArray &args)
{
  cwsh_->saveState();

  cwsh_->startBlock(BlockType::FUNCTION, lines_);

  cwsh_->defineVariable("argv", args);

  while (! cwsh_->blockEof()) {
    auto line = cwsh_->blockReadLine();

    cwsh_->processInputLine(line);

    if (cwsh_->isBlockReturn())
      break;
  }

  cwsh_->endBlock();

  cwsh_->setBlockReturn(false);

  cwsh_->restoreState();
}

void
Function::
list(bool all)
{
  auto *mgr = CwshMgrInst;

  std::cout << mgr->funcNameColorStr() << getName() << mgr->resetColorStr();

  if (all) {
    std::cout << " [";

    std::cout << mgr->locationColorStr() << getFilename() << ":" << getLineNum() <<
                 mgr->resetColorStr();

    std::cout << "]";
  }

  std::cout << "\n";
}

}
