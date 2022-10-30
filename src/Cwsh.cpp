#include <CwshI.h>
#include <CwshShMem.h>
#include <CCommandMgr.h>
#include <COSPty.h>
#include <CConfig.h>
#include <CArgs.h>
#include <CEscapeColors.h>
#include <CRGBName.h>
#include <cstdio>

namespace {

bool configColor(CConfig &config, const std::string &name, std::string &colorStr) {
  colorStr = "";

  std::string value;

  if (! config.getValue(name, value))
    return false;

  CRGBA c;

  if (! CRGBName::toRGBA(value, c))
    return false;

  colorStr = CEscapeColorsInst->colorFgStr(c);

  return true;
}

}

//---

CwshMgr &
CwshMgr::
getInstance()
{
  static CwshMgr *instance_;

  if (! instance_)
    instance_ = new CwshMgr();

  return *instance_;
}

CwshMgr::
CwshMgr() :
 config_("Cwsh")
{
  configColor(config_, "locationColor", locationColor_);

  configColor(config_, "aliasNameColor" , aliasNameColor_ );
  configColor(config_, "aliasValueColor", aliasValueColor_);

  configColor(config_, "envNameColor" , envNameColor_ );
  configColor(config_, "envValueColor", envValueColor_);

  configColor(config_, "funcNameColor" , funcNameColor_ );
  configColor(config_, "funcValueColor", funcValueColor_);

  configColor(config_, "helpNameColor", helpNameColor_);
  configColor(config_, "helpArgsColor", helpArgsColor_);
  configColor(config_, "helpDescColor", helpDescColor_);

  configColor(config_, "varNameColor" , varNameColor_ );
  configColor(config_, "varValueColor", varValueColor_);

  resetColor_ = "[0m";
}

CwshMgr::
~CwshMgr()
{
}

void
CwshMgr::
add(Cwsh *cwsh)
{
  cwshList_.push_back(cwsh);
}

void
CwshMgr::
remove(Cwsh *cwsh)
{
  cwshList_.remove(cwsh);
}

void
CwshMgr::
term(int status)
{
  auto p = cwshList_.begin();

  while (p != cwshList_.end()) {
    term(*p, status);

    remove(*p);

    p = cwshList_.begin();
  }
}

void
CwshMgr::
term(Cwsh *cwsh, int status)
{
  cwsh->term();

  if (cwshList_.size() == 0)
    exit(status);
}

void
CwshMgr::
setInterrupt(bool flag)
{
  for (auto &cwsh : cwshList_)
    cwsh->setInterrupt(flag);
}

void
CwshMgr::
readInterrupt()
{
  for (auto &cwsh : cwshList_)
    cwsh->readInterrupt();
}

void
CwshMgr::
gotoBlockLabel(const std::string &label)
{
  for (auto &cwsh : cwshList_)
    cwsh->gotoBlockLabel(label);
}

void
CwshMgr::
stopActiveProcesses()
{
  for (auto &cwsh : cwshList_) {
    CwshProcess *process = cwsh->getCurrentActiveProcess();

    if (process) {
      std::cout << "[" << process->getNum() << "]    Stopped               ";

      process->print();

      std::cout << "\n";

      process->tstop();
    }
  }
}

//--------

Cwsh::
Cwsh()
{
  functionMgr_ = new CwshFunctionMgr(this);
  variableMgr_ = new CwshVariableMgr(this);
  processMgr_  = new CwshProcessMgr(this);
  stateMgr_    = new CwshStateMgr(this);
  blockMgr_    = new CwshBlockMgr(this);
  aliasMgr_    = new CwshAliasMgr(this);
  autoExecMgr_ = new CwshAutoExecMgr(this);
  history_     = new CwshHistory(this);
  shellCmdMgr_ = new CwshShellCommandMgr(this);
  input_       = new CwshInput(this);
  readLine_    = new CwshReadLine(this);
  dirStack_    = new CwshDirStack;
  hash_        = new CwshHash(this);
  resource_    = new CwshResource;

#ifdef USE_SHM
  sh_mem_ = new CwshShMem();

  sh_mem_->purge();
#endif

  //readLine_->enableTimeoutHook();

  CwshMgrInst.add(this);
}

Cwsh::
~Cwsh()
{
  cleanup();

  CwshMgrInst.remove(this);
}

void
Cwsh::
cleanup()
{
  inputFile_   = nullptr;
  functionMgr_ = nullptr;
  variableMgr_ = nullptr;
  processMgr_  = nullptr;
  stateMgr_    = nullptr;
  blockMgr_    = nullptr;
  aliasMgr_    = nullptr;
  autoExecMgr_ = nullptr;
  history_     = nullptr;
  shellCmdMgr_ = nullptr;
  input_       = nullptr;
  readLine_    = nullptr;
  dirStack_    = nullptr;
  hash_        = nullptr;
  resource_    = nullptr;
  server_      = nullptr;
#ifdef USE_SHM
  sh_mem_      = nullptr;
#endif
}

void
Cwsh::
init()
{
  int   argc   = 1;
  char *argv[] = { const_cast<char *>("cwsh"), nullptr };

  init(argc, argv);
}

void
Cwsh::
init(int argc, char **argv)
{
  if (! processArgs(argc, argv))
    exit(1);

  initEnv();
}

bool
Cwsh::
processArgs(int argc, char **argv)
{
  std::string opts = "\
 -c:s           (Execute Following Commands) \
 -e:f           (Exit On Error) \
 -f:f           (Don't execute .cwshrc On Startup) \
 -i:f           (Interactive Mode) \
 -n:f           (No Execute Mode) \
 -s:f           (Command input from stdin) \
 -t:f           (Exit after executing command) \
 -v:f           (Verbose after execute of .cwshrc) \
 -V:f           (Verbose before execute of .cwshrc) \
 -x:f           (Echo after execute of .cwshrc) \
 -X:f           (Echo before execute of .cwshrc) \
 -h:f           (Display usage information) \
 --compatible:f (C Shell Compatible) \
 --silent:f     (Silent Mode) \
 --server:f     (Enable Server) \
 --debug:f      (Turn On Debug Messages) \
";

  CArgs cargs(opts);

  cargs.parse(&argc, argv);

  if (cargs.getBooleanArg("-h")) {
    cargs.usage(argv[0]);
    return false;
  }

  command_string_ = cargs.getStringArg ("-c");
  exitOnError_    = cargs.getBooleanArg("-e");
  fastStartup_    = cargs.getBooleanArg("-f");
  interactive_    = cargs.getBooleanArg("-i");
  noExecute_      = cargs.getBooleanArg("-n");
  exitAfterCmd_   = cargs.getBooleanArg("-t");
  verbose1_       = cargs.getBooleanArg("-v");
  verbose2_       = cargs.getBooleanArg("-V");
  echo1_          = cargs.getBooleanArg("-x");
  echo2_          = cargs.getBooleanArg("-X");
  compatible_     = cargs.getBooleanArg("--compatible");
  silentMode_     = cargs.getBooleanArg("--silent");
  debug_          = cargs.getBooleanArg("--debug");

  defineVariable("argv", const_cast<const char **>(&argv[1]), argc - 1);

  argv0_ = argv[0];

  loginShell_ = false;

  if (argv[0][0] == '-')
    loginShell_ = true;

  name_ = argv[0];

  if (argc > 1)
    initFilename_ = argv[1];

  if (cargs.getBooleanArg("--server"))
    enableServer();

  return true;
}

void
Cwsh::
initEnv()
{
  if (debug_)
    CCommandMgrInst->setDebug(true);

  if (initFilename_ != "")
    inputFile_ = new CFile(initFilename_);
  else {
    inputFile_ = new CFile(stdin);

    interactive_ = true;
  }

  defineVariable("cwsh", "0.1");
  defineVariable("filec");

  //------

  if (CEnvInst.exists("PATH")) {
    std::vector<std::string> values = CEnvInst.getValues("PATH");

    defineVariable("path", values);
  }

  defineVariable("user" , COSUser::getUserName       ());
  defineVariable("uid"  , COSUser::getEffectiveUserId());
  defineVariable("gid"  , COSUser::getUserGroupId    ());
  defineVariable("home" , COSUser::getUserHome       ());
  defineVariable("shell", COSUser::getUserShell      ());
  defineVariable("cwd"  , COSFile::getCurrentDir     ());

  promptType_    = CwshPromptType::NORMAL;
  promptCommand_ = "";

  if (interactive_)
    defineVariable("prompt", "> ");

  defineVariable("history", 20);

  defineVariable("status", 0);

  if (CEnvInst.exists("TERM")) {
    std::string term_env = CEnvInst.get("TERM");

    defineVariable("term", term_env);
  }

  //------

  if (CEnvInst.exists("CWSH_SILENT"))
    silentMode_ = true;

  //------

  CwshSignal::addHandlers();

  //------

  if (verbose2_)
    defineVariable("verbose");

  if (echo2_)
    defineVariable("echo");

  //------

  if (! fastStartup_)
    startup();

  //------

  if (verbose1_)
    defineVariable("verbose");

  if (echo1_)
    defineVariable("echo");

  //------

  if (command_string_ != "")
    processLine(command_string_);
}

void
Cwsh::
enableServer()
{
  server_ = new CwshServer(this);
}

CMessage *
Cwsh::
createServerMessage()
{
  return CwshServer::createMessage();
}

void
Cwsh::
mainLoop()
{
  if (! getExit())
    input_->execute(inputFile_);

  CwshMgrInst.term(this, 0);
}

void
Cwsh::
processLine(const std::string &line)
{
  processInputLine(line);
}

void
Cwsh::
startup()
{
  std::string home = COSUser::getUserHome();

  CDir dir(home);

  dir.enter();

  if (loginShell_) {
    if (CFile::exists(".login"))
      executeInput(".login");
  }

  if      (CFile::exists(".cwsh++"))
    executeInput(".cwsh++");
  else if (CFile::exists(".cwshrc"))
    executeInput(".cwshrc");
  else if (CFile::exists(".cshrc"))
    executeInput(".cshrc");

  dir.leave();
}

void
Cwsh::
term()
{
  if (getNumActiveProcesses() > 0 && termTries_ <= 0) {
    std::cerr << "There are suspended jobs.\n";
    ++termTries_;
    return;
  }

  // TODO: send all foreground and background process groups a SIGHUP, followed
  // by a SIGCONT signal (optional ?) if stopped

  if (loginShell_) {
    std::string home = COSUser::getUserHome();

    CDir dir(home);

    dir.enter();

    if (CFile::exists(".logout"))
      executeInput(".logout");

    dir.leave();
  }

  cleanup();
}

bool
Cwsh::
changeDir(const std::string &dirname)
{
  if (! CDir::changeDir(dirname))
    return false;

  defineVariable("cwd", COSFile::getCurrentDir());

#ifdef USE_SHM
  sh_mem_->setPath(COSUser::getCurrentDir().c_str());
#endif

  return true;
}

void
Cwsh::
setDebug(bool flag)
{
  debug_ = flag;
}

//---------------

CwshFunction *
Cwsh::
defineFunction(const CwshFunctionName &name, const CwshLineArray &lines)
{
  CwshFunction *function = functionMgr_->define(name, lines);

  function->setFilename(getFilename());
  function->setLineNum (getLineNum ());

  return function;
}

void
Cwsh::
undefineFunction(const CwshFunctionName &name)
{
  functionMgr_->undefine(name);
}

CwshFunction *
Cwsh::
lookupFunction(const CwshFunctionName &name)
{
  return functionMgr_->lookup(name);
}

void
Cwsh::
listAllFunctions()
{
  functionMgr_->listAll(/*all*/true);
}

//---------------

CwshVariable *
Cwsh::
defineVariable(const CwshVariableName &name)
{
  if (! variableMgr_)
    return nullptr;

  CwshVariable *variable = variableMgr_->define(name);

  variable->setFilename(getFilename());
  variable->setLineNum (getLineNum ());

  return variable;
}

CwshVariable *
Cwsh::
defineVariable(const CwshVariableName &name, const CwshVariableValue &value)
{
  if (! variableMgr_)
    return nullptr;

  CwshVariable *variable = variableMgr_->define(name, value);

  variable->setFilename(getFilename());
  variable->setLineNum (getLineNum ());

  return variable;
}

CwshVariable *
Cwsh::
defineVariable(const CwshVariableName &name, int value)
{
  if (! variableMgr_)
    return nullptr;

  CwshVariable *variable = variableMgr_->define(name, value);

  variable->setFilename(getFilename());
  variable->setLineNum (getLineNum ());

  return variable;
}

CwshVariable *
Cwsh::
defineVariable(const CwshVariableName &name, const CwshVariableValueArray &values)
{
  if (! variableMgr_)
    return nullptr;

  CwshVariable *variable = variableMgr_->define(name, values);

  variable->setFilename(getFilename());
  variable->setLineNum (getLineNum ());

  return variable;
}

CwshVariable *
Cwsh::
defineVariable(const CwshVariableName &name, const char **values, int num_values)
{
  if (! variableMgr_)
    return nullptr;

  CwshVariable *variable = variableMgr_->define(name, values, num_values);

  variable->setFilename(getFilename());
  variable->setLineNum (getLineNum ());

  return variable;
}

void
Cwsh::
undefineVariable(const CwshVariableName &name)
{
  if (! variableMgr_)
    return;

  variableMgr_->undefine(name);
}

CwshVariable *
Cwsh::
lookupVariable(const CwshVariableName &name) const
{
  if (variableMgr_)
    return variableMgr_->lookup(name);

  return nullptr;
}

CwshVariableList::iterator
Cwsh::
variablesBegin()
{
  assert(variableMgr_);

  return variableMgr_->variablesBegin();
}

CwshVariableList::iterator
Cwsh::
variablesEnd()
{
  assert(variableMgr_);

  return variableMgr_->variablesEnd();
}

void
Cwsh::
listVariables(bool all) const
{
  if (variableMgr_)
    variableMgr_->listVariables(all);
}

void
Cwsh::
saveVariables()
{
  if (variableMgr_)
    variableMgr_->save();
}

void
Cwsh::
restoreVariables()
{
  if (variableMgr_)
    variableMgr_->restore();
}

bool
Cwsh::
isEnvironmentVariableLower(const std::string &name)
{
  if (variableMgr_)
    return variableMgr_->isEnvironmentVariableLower(name);
  else
    return false;
}

bool
Cwsh::
isEnvironmentVariableUpper(const std::string &name)
{
  if (variableMgr_)
    return variableMgr_->isEnvironmentVariableUpper(name);
  else
    return false;
}

void
Cwsh::
updateEnvironmentVariable(CwshVariable *variable)
{
  if (variableMgr_)
    variableMgr_->updateEnvironmentVariable(variable);
}

//---------------

CwshProcess *
Cwsh::
addProcess(CwshCommandData *command)
{
  return processMgr_->add(command);
}

void
Cwsh::
removeProcess(CwshProcess *process)
{
  processMgr_->remove(process);
}

void
Cwsh::
killProcess(int pid, int signal)
{
  processMgr_->kill(pid, signal);
}

int
Cwsh::
getNumActiveProcesses()
{
  if (! processMgr_) return 0;

  return processMgr_->getNumActive();
}

void
Cwsh::
displayActiveProcesses(bool list_pids)
{
  processMgr_->displayActive(list_pids);
}

void
Cwsh::
displayExitedProcesses()
{
  processMgr_->displayExited();
}

void
Cwsh::
waitActiveProcesses()
{
  processMgr_->waitActive();
}

int
Cwsh::
stringToProcessId(const std::string &str)
{
  return processMgr_->stringToPid(str);
}

CwshProcess *
Cwsh::
getActiveProcess(const std::string &str)
{
  return processMgr_->getActiveProcess(str);
}

CwshProcess *
Cwsh::
getCurrentActiveProcess()
{
  return processMgr_->getCurrentActiveProcess();
}

CwshProcess *
Cwsh::
lookupProcess(pid_t pid)
{
  return processMgr_->lookupProcess(pid);
}

//---------------

void
Cwsh::
saveState()
{
  stateMgr_->save(this);
}

void
Cwsh::
restoreState()
{
  stateMgr_->restore();
}

//---------------

CwshBlock *
Cwsh::
startBlock(CwshBlockType type, const CwshLineArray &lines)
{
  return blockMgr_->startBlock(type, lines);
}

void
Cwsh::
endBlock()
{
  blockMgr_->endBlock();
}

bool
Cwsh::
inBlock() const
{
  return blockMgr_->inBlock();
}

bool
Cwsh::
blockEof() const
{
  return blockMgr_->eof();
}

CwshLine
Cwsh::
blockReadLine() const
{
  return blockMgr_->readLine();
}

CwshBlock *
Cwsh::
findBlock(CwshBlockType type)
{
  return blockMgr_->find(type);
}

void
Cwsh::
gotoBlockLabel(const std::string &label)
{
  blockMgr_->gotoLabel(label);
}

bool
Cwsh::
isBlockBreak() const
{
  return blockMgr_->isBreak();
}

bool
Cwsh::
isBlockBreakSwitch() const
{
  return blockMgr_->isBreakSwitch();
}

bool
Cwsh::
isBlockContinue() const
{
  return blockMgr_->isContinue();
}

bool
Cwsh::
isBlockReturn() const
{
  return blockMgr_->isReturn();
}

int
Cwsh::
getBlockGotoDepth() const
{
  return blockMgr_->getGotoDepth();
}

void
Cwsh::
setBlockBreak(bool flag)
{
  return blockMgr_->setBreak(flag);
}

void
Cwsh::
setBlockBreakSwitch(bool flag)
{
  return blockMgr_->setBreakSwitch(flag);
}

void
Cwsh::
setBlockContinue(bool flag)
{
  return blockMgr_->setContinue(flag);
}

void
Cwsh::
setBlockReturn(bool flag)
{
  return blockMgr_->setReturn(flag);
}

//---------------

CwshAlias *
Cwsh::
defineAlias(const CwshAliasName &name, const CwshAliasValue &value)
{
  CwshAlias *alias = aliasMgr_->define(name, value);

  alias->setFilename(getFilename());
  alias->setLineNum (getLineNum ());

  return alias;
}

void
Cwsh::
undefineAlias(const CwshAliasName &name)
{
  aliasMgr_->undefine(name);
}

CwshAlias *
Cwsh::
lookupAlias(const CwshAliasName &name) const
{
  return aliasMgr_->lookup(name);
}

bool
Cwsh::
substituteAlias(CwshCmd *cmd, CwshCmdArray &cmds) const
{
  return aliasMgr_->substitute(cmd, cmds);
}

void
Cwsh::
displayAliases(bool all) const
{
  aliasMgr_->display(all);
}

//---------------

void
Cwsh::
defineAutoExec(const CwshAutoExecName &name, const CwshAutoExecValue &value)
{
  autoExecMgr_->define(name, value);
}

void
Cwsh::
undefineAutoExec(const CwshAutoExecName &name)
{
  autoExecMgr_->undefine(name);
}

CwshAutoExec *
Cwsh::
lookupAutoExec(const CwshAutoExecName &name) const
{
  return autoExecMgr_->lookup(name);
}

void
Cwsh::
displayAutoExec() const
{
  autoExecMgr_->display();
}

//---------------

int
Cwsh::
getHistoryCommandNum() const
{
  return history_->getCommandNum();
}

bool
Cwsh::
findHistoryCommandStart(const std::string &text, int &command_num)
{
  return history_->findCommandStart(text, command_num);
}

bool
Cwsh::
findHistoryCommandIn(const std::string &text, int &command_num)
{
  return history_->findCommandIn(text, command_num);
}

bool
Cwsh::
findHistoryCommandArg(const std::string &text, int &command_num, int &arg_num)
{
  return history_->findCommandArg(text, command_num, arg_num);
}

std::string
Cwsh::
getHistoryCommand(int num)
{
  return history_->getCommand(num);
}

std::string
Cwsh::
getHistoryCommandArg(int num, int arg_num)
{
  return history_->getCommandArg(num, arg_num);
}

void
Cwsh::
addHistoryFile(const std::string &filename)
{
  history_->addFile(filename);
}

void
Cwsh::
addHistoryCommand(const std::string &text)
{
  history_->addCommand(text);
}

void
Cwsh::
setHistoryCurrent(const std::string &text)
{
  history_->setCurrent(text);
}

void
Cwsh::
displayHistory(int num, bool show_numbers, bool show_time, bool reverse)
{
  history_->display(num, show_numbers, show_time, reverse);
}

bool
Cwsh::
hasPrevHistoryCommand()
{
  return history_->hasPrevCommand();
}

bool
Cwsh::
hasNextHistoryCommand()
{
  return history_->hasNextCommand();
}

std::string
Cwsh::
getPrevHistoryCommand()
{
  return history_->getPrevCommand();
}

std::string
Cwsh::
getNextHistoryCommand()
{
  return history_->getNextCommand();
}

//------------

CwshShellCommand *
Cwsh::
lookupShellCommand(const std::string &name) const
{
  return shellCmdMgr_->lookup(name);
}

//------------

void
Cwsh::
executeInput(const std::string &filename)
{
  input_->execute(filename);
}

void
Cwsh::
processInputLine(const CwshLine &line)
{
  input_->processLine(line);
}

void
Cwsh::
getInputBlock(CwshShellCommand *command, CwshLineArray &lines)
{
  input_->getBlock(command, lines);
}

void
Cwsh::
skipInputBlock(const CwshLine &line)
{
  input_->skipBlock(line);
}

bool
Cwsh::
inputEof()
{
  return input_->eof();
}

CwshLine
Cwsh::
getInputLine()
{
  return input_->getLine();
}

std::string
Cwsh::
getInputPrompt()
{
  if (getSilentMode())
    return "";

  return input_->getPrompt();
}

std::string
Cwsh::
processInputExprLine(const CwshLine &line)
{
  return input_->processExprLine(line);
}

//------------

std::string
Cwsh::
readLine()
{
  //struct termios t;

  //if (getSilentMode()) COSPty::set_raw(0, &t);

  std::string str = readLine_->readLine();

  //if (getSilentMode()) COSPty::reset_raw(0, &t);

  return str;
}

void
Cwsh::
beep()
{
  readLine_->beep();
}

void
Cwsh::
readInterrupt()
{
  readLine_->interrupt();
}

//------------

void
Cwsh::
pushDirStack()
{
  dirStack_->push();
}

void
Cwsh::
pushDirStack(const std::string &dirname)
{
  dirStack_->push(dirname);
}

std::string
Cwsh::
popDirStack()
{
  return dirStack_->pop();
}

std::string
Cwsh::
popDirStack(int pos)
{
  return dirStack_->pop(pos);
}

int
Cwsh::
sizeDirStack()
{
  return dirStack_->size();
}

void
Cwsh::
printDirStack(bool /*expand_home*/)
{
  dirStack_->print();
}

//------------

void
Cwsh::
addFilePath(const std::string &filename, const std::string &path)
{
  hash_->addFilePath(filename, path);
}

std::string
Cwsh::
getFilePath(const std::string &filename)
{
  return hash_->getFilePath(filename);
}

void
Cwsh::
clearFilePath()
{
  hash_->clearFilePath();
}

void
Cwsh::
printFilePathStats()
{
  hash_->printFilePathStats();
}

void
Cwsh::
setFilePathActive(bool flag)
{
  hash_->setFilePathActive(flag);
}

//------------

void
Cwsh::
limitResource(const std::string &name, const std::string &value, bool hard)
{
  resource_->limit(name, value, hard);
}

void
Cwsh::
unlimitAllResources()
{
  resource_->unlimitAll();
}

void
Cwsh::
unlimitResource(const std::string &name)
{
  resource_->unlimit(name);
}

void
Cwsh::
printAllResources(bool hard)
{
  resource_->printAll(hard);
}

void
Cwsh::
printResource(const std::string &name, bool hard)
{
  resource_->print(name, hard);
}

void
Cwsh::
readTimeout()
{
#if 0
  std::string line = readLine_->getBuffer();

  line = colorLine(line);

  readLine_->setBuffer(line);
#endif

  if (server_)
    server_->processMessage();
}

std::string
Cwsh::
colorLine(const std::string &line)
{
  return "[1m" + line + "[0m";
}

std::string
Cwsh::
getAliasesMsg() const
{
  return aliasMgr_->getAliasesMsg();
}

std::string
Cwsh::
getHistoryMsg() const
{
  return history_->getHistoryMsg();
}
