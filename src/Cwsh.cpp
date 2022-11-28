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

namespace Cwsh {

Mgr *
Mgr::
getInstance()
{
  static Mgr *instance_;

  if (! instance_)
    instance_ = new Mgr();

  return instance_;
}

Mgr::
Mgr() :
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

Mgr::
~Mgr()
{
}

void
Mgr::
add(App *cwsh)
{
  cwshList_.push_back(cwsh);
}

void
Mgr::
remove(App *cwsh)
{
  cwshList_.remove(cwsh);
}

void
Mgr::
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
Mgr::
term(App *cwsh, int status)
{
  cwsh->term();

  if (cwshList_.size() == 0)
    exit(status);
}

void
Mgr::
setInterrupt(bool flag)
{
  for (auto &cwsh : cwshList_)
    cwsh->setInterrupt(flag);
}

void
Mgr::
readInterrupt()
{
  for (auto &cwsh : cwshList_)
    cwsh->readInterrupt();
}

void
Mgr::
gotoBlockLabel(const std::string &label)
{
  for (auto &cwsh : cwshList_)
    cwsh->gotoBlockLabel(label);
}

void
Mgr::
stopActiveProcesses()
{
  for (auto &cwsh : cwshList_) {
    auto *process = cwsh->getCurrentActiveProcess();

    if (process) {
      std::cout << "[" << process->getNum() << "]    Stopped               ";

      process->print();

      std::cout << "\n";

      process->tstop();
    }
  }
}

//--------

App::
App()
{
  functionMgr_ = std::make_unique<FunctionMgr    >(this);
  variableMgr_ = std::make_unique<VariableMgr    >(this);
  processMgr_  = std::make_unique<ProcessMgr     >(this);
  stateMgr_    = std::make_unique<StateMgr       >(this);
  blockMgr_    = std::make_unique<BlockMgr       >(this);
  aliasMgr_    = std::make_unique<AliasMgr       >(this);
  autoExecMgr_ = std::make_unique<AutoExecMgr    >(this);
  history_     = std::make_unique<History        >(this);
  shellCmdMgr_ = std::make_unique<ShellCommandMgr>(this);
  input_       = std::make_unique<Input          >(this);
  readLine_    = std::make_unique<ReadLine       >(this);
  dirStack_    = std::make_unique<DirStack       >();
  hash_        = std::make_unique<Hash           >(this);
  resource_    = std::make_unique<Resource       >();

#ifdef USE_SHM
  sh_mem_ = std::make_unique<ShMem>();

  sh_mem_->purge();
#endif

  //readLine_->enableTimeoutHook();

  auto *mgr = CwshMgrInst;

  mgr->add(this);
}

App::
~App()
{
  cleanup();

  auto *mgr = CwshMgrInst;

  mgr->remove(this);
}

void
App::
cleanup()
{
  inputFile_   = FileP();
  functionMgr_ = FunctionMgrP();
  variableMgr_ = VariableMgrP();
  processMgr_  = ProcessMgrP();
  stateMgr_    = StateMgrP();
  blockMgr_    = BlockMgrP();
  aliasMgr_    = AliasMgrP();
  autoExecMgr_ = AutoExecMgrP();
  history_     = HistoryP();
  shellCmdMgr_ = ShellCommandMgrP();
  input_       = InputP();
  readLine_    = ReadLineP();
  dirStack_    = DirStackP();
  hash_        = HashP();
  resource_    = ResourceP();
  server_      = ServerP();
#ifdef USE_SHM
  sh_mem_      = ShMemP();
#endif
}

void
App::
init()
{
  int   argc   = 1;
  char *argv[] = { const_cast<char *>("cwsh"), nullptr };

  init(argc, argv);
}

void
App::
init(int argc, char **argv)
{
  if (! processArgs(argc, argv))
    exit(1);

  initEnv();
}

bool
App::
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

  commandString_ = cargs.getStringArg ("-c");
  exitOnError_   = cargs.getBooleanArg("-e");
  fastStartup_   = cargs.getBooleanArg("-f");
  interactive_   = cargs.getBooleanArg("-i");
  noExecute_     = cargs.getBooleanArg("-n");
  exitAfterCmd_  = cargs.getBooleanArg("-t");
  verbose1_      = cargs.getBooleanArg("-v");
  verbose2_      = cargs.getBooleanArg("-V");
  echo1_         = cargs.getBooleanArg("-x");
  echo2_         = cargs.getBooleanArg("-X");
  compatible_    = cargs.getBooleanArg("--compatible");
  silentMode_    = cargs.getBooleanArg("--silent");
  debug_         = cargs.getBooleanArg("--debug");

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
App::
initEnv()
{
  if (debug_)
    CCommandMgrInst->setDebug(true);

  if (initFilename_ != "")
    inputFile_ = std::make_unique<CFile>(initFilename_);
  else {
    inputFile_ = std::make_unique<CFile>(stdin);

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

  promptType_    = PromptType::NORMAL;
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

  Signal::addHandlers();

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

  if (commandString_ != "")
    processLine(commandString_);
}

void
App::
enableServer()
{
  server_ = std::make_unique<Server>(this);
}

App::MessageP
App::
createServerMessage()
{
  return Server::createMessage();
}

void
App::
mainLoop()
{
  if (! getExit())
    input_->execute(inputFile_.get());

  auto *mgr = CwshMgrInst;

  mgr->term(this, 0);
}

void
App::
processLine(const std::string &line)
{
  processInputLine(line);
}

void
App::
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
App::
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
App::
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
App::
setDebug(bool flag)
{
  debug_ = flag;
}

//---------------

Function *
App::
defineFunction(const std::string &name, const LineArray &lines)
{
  auto *function = functionMgr_->define(name, lines);

  function->setFilename(getFilename());
  function->setLineNum (getLineNum ());

  return function;
}

void
App::
undefineFunction(const std::string &name)
{
  functionMgr_->undefine(name);
}

Function *
App::
lookupFunction(const std::string &name)
{
  return functionMgr_->lookup(name);
}

void
App::
listAllFunctions()
{
  functionMgr_->listAll(/*all*/true);
}

//---------------

Variable *
App::
defineVariable(const std::string &name)
{
  if (! variableMgr_)
    return nullptr;

  auto *variable = variableMgr_->define(name);

  variable->setFilename(getFilename());
  variable->setLineNum (getLineNum ());

  return variable;
}

Variable *
App::
defineVariable(const std::string &name, const std::string &value)
{
  if (! variableMgr_)
    return nullptr;

  auto *variable = variableMgr_->define(name, value);

  variable->setFilename(getFilename());
  variable->setLineNum (getLineNum ());

  return variable;
}

Variable *
App::
defineVariable(const std::string &name, int value)
{
  if (! variableMgr_)
    return nullptr;

  auto *variable = variableMgr_->define(name, value);

  variable->setFilename(getFilename());
  variable->setLineNum (getLineNum ());

  return variable;
}

Variable *
App::
defineVariable(const std::string &name, const VariableValueArray &values)
{
  if (! variableMgr_)
    return nullptr;

  auto *variable = variableMgr_->define(name, values);

  variable->setFilename(getFilename());
  variable->setLineNum (getLineNum ());

  return variable;
}

Variable *
App::
defineVariable(const std::string &name, const char **values, int numValues)
{
  if (! variableMgr_)
    return nullptr;

  auto *variable = variableMgr_->define(name, values, numValues);

  variable->setFilename(getFilename());
  variable->setLineNum (getLineNum ());

  return variable;
}

void
App::
undefineVariable(const std::string &name)
{
  if (! variableMgr_)
    return;

  variableMgr_->undefine(name);
}

Variable *
App::
lookupVariable(const std::string &name) const
{
  if (variableMgr_)
    return variableMgr_->lookup(name);

  return nullptr;
}

const VariableList &
App::
variables() const
{
  assert(variableMgr_);

  return variableMgr_->variables();
}

void
App::
listVariables(bool all) const
{
  if (variableMgr_)
    variableMgr_->listVariables(all);
}

void
App::
saveVariables()
{
  if (variableMgr_)
    variableMgr_->save();
}

void
App::
restoreVariables()
{
  if (variableMgr_)
    variableMgr_->restore();
}

bool
App::
isEnvironmentVariableLower(const std::string &name)
{
  if (variableMgr_)
    return variableMgr_->isEnvironmentVariableLower(name);
  else
    return false;
}

bool
App::
isEnvironmentVariableUpper(const std::string &name)
{
  if (variableMgr_)
    return variableMgr_->isEnvironmentVariableUpper(name);
  else
    return false;
}

void
App::
updateEnvironmentVariable(Variable *variable)
{
  if (variableMgr_)
    variableMgr_->updateEnvironmentVariable(variable);
}

//---------------

Process *
App::
addProcess(CommandData *command)
{
  return processMgr_->add(command);
}

void
App::
removeProcess(Process *process)
{
  processMgr_->remove(process);
}

void
App::
killProcess(int pid, int signal)
{
  processMgr_->kill(pid, signal);
}

int
App::
getNumActiveProcesses()
{
  if (! processMgr_) return 0;

  return processMgr_->getNumActive();
}

void
App::
displayActiveProcesses(bool listPids)
{
  processMgr_->displayActive(listPids);
}

void
App::
displayExitedProcesses()
{
  processMgr_->displayExited();
}

void
App::
waitActiveProcesses()
{
  processMgr_->waitActive();
}

int
App::
stringToProcessId(const std::string &str)
{
  return processMgr_->stringToPid(str);
}

Process *
App::
getActiveProcess(const std::string &str)
{
  return processMgr_->getActiveProcess(str);
}

Process *
App::
getCurrentActiveProcess()
{
  return processMgr_->getCurrentActiveProcess();
}

Process *
App::
lookupProcess(pid_t pid)
{
  return processMgr_->lookupProcess(pid);
}

//---------------

void
App::
saveState()
{
  stateMgr_->save(this);
}

void
App::
restoreState()
{
  stateMgr_->restore();
}

//---------------

Block *
App::
startBlock(BlockType type, const LineArray &lines)
{
  return blockMgr_->startBlock(type, lines);
}

void
App::
endBlock()
{
  blockMgr_->endBlock();
}

bool
App::
inBlock() const
{
  return blockMgr_->inBlock();
}

bool
App::
blockEof() const
{
  return blockMgr_->eof();
}

Line
App::
blockReadLine() const
{
  return blockMgr_->readLine();
}

Block *
App::
findBlock(BlockType type)
{
  return blockMgr_->find(type);
}

void
App::
gotoBlockLabel(const std::string &label)
{
  blockMgr_->gotoLabel(label);
}

bool
App::
isBlockBreak() const
{
  return blockMgr_->isBreak();
}

bool
App::
isBlockBreakSwitch() const
{
  return blockMgr_->isBreakSwitch();
}

bool
App::
isBlockContinue() const
{
  return blockMgr_->isContinue();
}

bool
App::
isBlockReturn() const
{
  return blockMgr_->isReturn();
}

int
App::
getBlockGotoDepth() const
{
  return blockMgr_->getGotoDepth();
}

void
App::
setBlockBreak(bool flag)
{
  return blockMgr_->setBreak(flag);
}

void
App::
setBlockBreakSwitch(bool flag)
{
  return blockMgr_->setBreakSwitch(flag);
}

void
App::
setBlockContinue(bool flag)
{
  return blockMgr_->setContinue(flag);
}

void
App::
setBlockReturn(bool flag)
{
  return blockMgr_->setReturn(flag);
}

//---------------

Alias *
App::
defineAlias(const std::string &name, const std::string &value)
{
  auto *alias = aliasMgr_->define(name, value);

  alias->setFilename(getFilename());
  alias->setLineNum (getLineNum ());

  return alias;
}

void
App::
undefineAlias(const std::string &name)
{
  aliasMgr_->undefine(name);
}

Alias *
App::
lookupAlias(const std::string &name) const
{
  return aliasMgr_->lookup(name);
}

bool
App::
substituteAlias(Cmd *cmd, CmdArray &cmds) const
{
  return aliasMgr_->substitute(cmd, cmds);
}

void
App::
displayAliases(bool all) const
{
  aliasMgr_->display(all);
}

//---------------

void
App::
defineAutoExec(const std::string &name, const std::string &value)
{
  autoExecMgr_->define(name, value);
}

void
App::
undefineAutoExec(const std::string &name)
{
  autoExecMgr_->undefine(name);
}

AutoExec *
App::
lookupAutoExec(const std::string &name) const
{
  return autoExecMgr_->lookup(name);
}

void
App::
displayAutoExec() const
{
  autoExecMgr_->display();
}

//---------------

int
App::
getHistoryCommandNum() const
{
  return history_->getCommandNum();
}

bool
App::
findHistoryCommandStart(const std::string &text, int &commandNum)
{
  return history_->findCommandStart(text, commandNum);
}

bool
App::
findHistoryCommandIn(const std::string &text, int &commandNum)
{
  return history_->findCommandIn(text, commandNum);
}

bool
App::
findHistoryCommandArg(const std::string &text, int &commandNum, int &argNum)
{
  return history_->findCommandArg(text, commandNum, argNum);
}

std::string
App::
getHistoryCommand(int num)
{
  return history_->getCommand(num);
}

std::string
App::
getHistoryCommandArg(int num, int argNum)
{
  return history_->getCommandArg(num, argNum);
}

void
App::
addHistoryFile(const std::string &filename)
{
  history_->addFile(filename);
}

void
App::
addHistoryCommand(const std::string &text)
{
  history_->addCommand(text);
}

void
App::
setHistoryCurrent(const std::string &text)
{
  history_->setCurrent(text);
}

void
App::
displayHistory(int num, bool showNumbers, bool showTime, bool reverse)
{
  history_->display(num, showNumbers, showTime, reverse);
}

bool
App::
hasPrevHistoryCommand()
{
  return history_->hasPrevCommand();
}

bool
App::
hasNextHistoryCommand()
{
  return history_->hasNextCommand();
}

std::string
App::
getPrevHistoryCommand()
{
  return history_->getPrevCommand();
}

std::string
App::
getNextHistoryCommand()
{
  return history_->getNextCommand();
}

//------------

ShellCommand *
App::
lookupShellCommand(const std::string &name) const
{
  return shellCmdMgr_->lookup(name);
}

//------------

void
App::
executeInput(const std::string &filename)
{
  input_->execute(filename);
}

void
App::
processInputLine(const Line &line)
{
  input_->processLine(line);
}

void
App::
getInputBlock(ShellCommand *command, LineArray &lines)
{
  input_->getBlock(command, lines);
}

void
App::
skipInputBlock(const Line &line)
{
  input_->skipBlock(line);
}

bool
App::
inputEof()
{
  return input_->eof();
}

Line
App::
getInputLine()
{
  return input_->getLine();
}

std::string
App::
getInputPrompt()
{
  if (getSilentMode())
    return "";

  return input_->getPrompt();
}

std::string
App::
processInputExprLine(const Line &line)
{
  return input_->processExprLine(line);
}

//------------

std::string
App::
readLine()
{
  //struct termios t;

  //if (getSilentMode()) COSPty::set_raw(0, &t);

  std::string str = readLine_->readLine();

  //if (getSilentMode()) COSPty::reset_raw(0, &t);

  return str;
}

void
App::
beep()
{
  readLine_->beep();
}

void
App::
readInterrupt()
{
  readLine_->interrupt();
}

//------------

void
App::
pushDirStack()
{
  dirStack_->push();
}

void
App::
pushDirStack(const std::string &dirname)
{
  dirStack_->push(dirname);
}

std::string
App::
popDirStack()
{
  return dirStack_->pop();
}

std::string
App::
popDirStack(int pos)
{
  return dirStack_->pop(pos);
}

int
App::
sizeDirStack()
{
  return dirStack_->size();
}

void
App::
printDirStack(bool /*expandHome*/)
{
  dirStack_->print();
}

//------------

void
App::
addFilePath(const std::string &filename, const std::string &path)
{
  hash_->addFilePath(filename, path);
}

std::string
App::
getFilePath(const std::string &filename)
{
  return hash_->getFilePath(filename);
}

void
App::
clearFilePath()
{
  hash_->clearFilePath();
}

void
App::
printFilePathStats()
{
  hash_->printFilePathStats();
}

void
App::
setFilePathActive(bool flag)
{
  hash_->setFilePathActive(flag);
}

//------------

void
App::
limitResource(const std::string &name, const std::string &value, bool hard)
{
  resource_->limit(name, value, hard);
}

void
App::
unlimitAllResources()
{
  resource_->unlimitAll();
}

void
App::
unlimitResource(const std::string &name)
{
  resource_->unlimit(name);
}

void
App::
printAllResources(bool hard)
{
  resource_->printAll(hard);
}

void
App::
printResource(const std::string &name, bool hard)
{
  resource_->print(name, hard);
}

void
App::
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
App::
colorLine(const std::string &line)
{
  return "[1m" + line + "[0m";
}

std::string
App::
getAliasesMsg() const
{
  return aliasMgr_->getAliasesMsg();
}

std::string
App::
getHistoryMsg() const
{
  return history_->getHistoryMsg();
}

}
