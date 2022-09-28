#include <CwshI.h>

CwshFunctionMgr::
CwshFunctionMgr(Cwsh *cwsh) :
 cwsh_(cwsh)
{
}

CwshFunction *
CwshFunctionMgr::
define(const CwshFunctionName &name, const CwshLineArray &lines)
{
  CwshFunction *function = new CwshFunction(cwsh_, name, lines);

  function_list_.setValue(name, function);

  return function;
}

void
CwshFunctionMgr::
undefine(const CwshFunctionName &name)
{
  function_list_.unsetValue(name);
}

CwshFunction *
CwshFunctionMgr::
lookup(const CwshFunctionName &name)
{
  return function_list_.getValue(name);
}

void
CwshFunctionMgr::
listAll(bool all)
{
  for (const auto &pfunction : function_list_) {
    CwshFunction *function = pfunction.second;

    function->list(all);
  }
}

//------

CwshFunction::
CwshFunction(Cwsh *cwsh, const CwshFunctionName &name, const CwshLineArray &lines) :
 cwsh_(cwsh), name_(name), lines_(lines)
{
}

CwshFunction::
~CwshFunction()
{
}
void
CwshFunction::
runProc(const CwshArgArray &args, CCommand::CallbackData data)
{
  auto *function = reinterpret_cast<CwshFunction *>(data);

  function->run(args);
}

void
CwshFunction::
run(const CwshArgArray &args)
{
  cwsh_->saveState();

  cwsh_->startBlock(CwshBlockType::FUNCTION, lines_);

  cwsh_->defineVariable("argv", args);

  while (! cwsh_->blockEof()) {
    CwshLine line = cwsh_->blockReadLine();

    cwsh_->processInputLine(line);

    if (cwsh_->isBlockReturn())
      break;
  }

  cwsh_->endBlock();

  cwsh_->setBlockReturn(false);

  cwsh_->restoreState();
}

void
CwshFunction::
list(bool all)
{
  std::cout << CwshMgrInst.funcNameColorStr() << getName() << CwshMgrInst.resetColorStr();

  if (all) {
    std::cout << " [";

    std::cout << CwshMgrInst.locationColorStr() << getFilename() << ":" << getLineNum() <<
                 CwshMgrInst.resetColorStr();

    std::cout << "]";
  }

  std::cout << std::endl;
}

