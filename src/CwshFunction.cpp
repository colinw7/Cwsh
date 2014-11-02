#include <CwshI.h>

CwshFunctionMgr::
CwshFunctionMgr(Cwsh *cwsh) :
 cwsh_(cwsh)
{
}

void
CwshFunctionMgr::
define(const CwshFunctionName &name, const CwshLineArray &lines)
{
  CwshFunction *function = new CwshFunction(cwsh_, name, lines);

  function_list_.setValue(name, function);
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
  CwshFunction *function = (CwshFunction *) data;

  function->run(args);
}

void
CwshFunction::
run(const CwshArgArray &args)
{
  cwsh_->saveState();

  cwsh_->startBlock(CWSH_BLOCK_TYPE_FUNCTION, lines_);

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
CwshFunctionMgr::
listAll()
{
  FunctionList::iterator pfunction1 = function_list_.begin();
  FunctionList::iterator pfunction2 = function_list_.end  ();

  for ( ; pfunction1 != pfunction2; ++pfunction1) {
    CwshFunction *function = (*pfunction1).second;

    std::cout << function->getName() << std::endl;
  }
}
