#include <CwshI.h>
#include <CwshShMem.h>
#include <CArgs.h>
#include <CCommandMgr.h>
//#include <COSPty.h>
#include <cstdio>

CwshMgr &
CwshMgr::
getInstance()
{
  static CwshMgr *instance_;

  if (instance_ == NULL)
    instance_ = new CwshMgr();

  return *instance_;
}

CwshMgr::
CwshMgr()
{
}

CwshMgr::
~CwshMgr()
{
}

void
CwshMgr::
add(Cwsh *cwsh)
{
  cwsh_list_.push_back(cwsh);
}

void
CwshMgr::
remove(Cwsh *cwsh)
{
  cwsh_list_.remove(cwsh);
}

void
CwshMgr::
term(int status)
{
  CwshList::iterator p = cwsh_list_.begin();

  while (p != cwsh_list_.end()) {
    term(*p, status);

    remove(*p);

    p = cwsh_list_.begin();
  }
}

void
CwshMgr::
term(Cwsh *cwsh, int status)
{
  cwsh->term();

  if (cwsh_list_.size() == 0)
    exit(status);
}

void
CwshMgr::
setInterrupt(bool flag)
{
  CwshList::iterator p1, p2;

  for (p1 = cwsh_list_.begin(), p2 = cwsh_list_.end(); p1 != p2; ++p1)
    (*p1)->setInterrupt(flag);
}

void
CwshMgr::
readInterrupt()
{
  CwshList::iterator p1, p2;

  for (p1 = cwsh_list_.begin(), p2 = cwsh_list_.end(); p1 != p2; ++p1)
    (*p1)->readInterrupt();
}

void
CwshMgr::
gotoBlockLabel(const string &label)
{
  CwshList::iterator p1, p2;

  for (p1 = cwsh_list_.begin(), p2 = cwsh_list_.end(); p1 != p2; ++p1)
    (*p1)->gotoBlockLabel(label);
}

void
CwshMgr::
stopActiveProcesses()
{
  CwshList::iterator p1, p2;

  for (p1 = cwsh_list_.begin(), p2 = cwsh_list_.end(); p1 != p2; ++p1) {
    CwshProcess *process = (*p1)->getCurrentActiveProcess();

    if (process != NULL) {
      std::cout << "[" << process->getNum() << "]    Stopped               ";

      process->print();

      std::cout << std::endl;

      process->tstop();
    }
  }
}

//--------

Cwsh::
Cwsh() :
 exit_on_error_ (false),
 fast_startup_  (false),
 interactive_   (false),
 no_execute_    (false),
 exit_after_cmd_(false),
 login_shell_   (false),
 prompt_type_   (CWSH_PROMPT_TYPE_NORMAL),
 compatible_    (false),
 silentMode_    (false),
 debug_         (false),
 interrupt_     (false),
 verbose1_      (false),
 verbose2_      (false),
 echo1_         (false),
 echo2_         (false),
 term_tries_    (0),
 exit_          (false),
 exit_status_   (0)
{
  function_mgr_  = new CwshFunctionMgr(this);
  variable_mgr_  = new CwshVariableMgr(this);
  process_mgr_   = new CwshProcessMgr(this);
  state_mgr_     = new CwshStateMgr(this);
  block_mgr_     = new CwshBlockMgr(this);
  alias_mgr_     = new CwshAliasMgr(this);
  auto_exec_mgr_ = new CwshAutoExecMgr(this);
  history_       = new CwshHistory(this);
  shell_cmd_mgr_ = new CwshShellCommandMgr(this);
  input_         = new CwshInput(this);
  read_line_     = new CwshReadLine(this);
  dir_stack_     = new CwshDirStack;
  hash_          = new CwshHash(this);
  resource_      = new CwshResource;

#ifdef USE_SHM
  sh_mem_ = new CwshShMem();

  sh_mem_->purge();
#endif

  //read_line_->enableTimeoutHook();

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
  input_file_    = NULL;
  function_mgr_  = NULL;
  variable_mgr_  = NULL;
  process_mgr_   = NULL;
  state_mgr_     = NULL;
  block_mgr_     = NULL;
  alias_mgr_     = NULL;
  auto_exec_mgr_ = NULL;
  history_       = NULL;
  shell_cmd_mgr_ = NULL;
  input_         = NULL;
  read_line_     = NULL;
  dir_stack_     = NULL;
  hash_          = NULL;
  resource_      = NULL;
  server_        = NULL;
#ifdef USE_SHM
  sh_mem_        = NULL;
#endif
}

void
Cwsh::
init()
{
  int   argc   = 1;
  char *argv[] = { (char *) "cwsh", NULL };

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
  string opts = "\
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
  exit_on_error_  = cargs.getBooleanArg("-e");
  fast_startup_   = cargs.getBooleanArg("-f");
  interactive_    = cargs.getBooleanArg("-i");
  no_execute_     = cargs.getBooleanArg("-n");
  exit_after_cmd_ = cargs.getBooleanArg("-t");
  verbose1_       = cargs.getBooleanArg("-v");
  verbose2_       = cargs.getBooleanArg("-V");
  echo1_          = cargs.getBooleanArg("-x");
  echo2_          = cargs.getBooleanArg("-X");
  compatible_     = cargs.getBooleanArg("--compatible");
  silentMode_     = cargs.getBooleanArg("--silent");
  debug_          = cargs.getBooleanArg("--debug");

  defineVariable("argv", (const char **) &argv[1], argc - 1);

  argv0_ = argv[0];

  login_shell_ = false;

  if (argv[0][0] == '-')
    login_shell_ = true;

  name_ = argv[0];

  if (argc > 1)
    init_filename_ = argv[1];

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

  if (init_filename_ != "")
    input_file_ = new CFile(init_filename_);
  else {
    input_file_ = new CFile(stdin);

    interactive_ = true;
  }

  defineVariable("cwsh", "0.1");
  defineVariable("filec");

  //------

  if (CEnvInst.exists("PATH")) {
    vector<string> values = CEnvInst.getValues("PATH");

    defineVariable("path", values);
  }

  defineVariable("user" , COSUser::getUserName       ());
  defineVariable("uid"  , COSUser::getEffectiveUserId());
  defineVariable("gid"  , COSUser::getUserGroupId    ());
  defineVariable("home" , COSUser::getUserHome       ());
  defineVariable("shell", COSUser::getUserShell      ());
  defineVariable("cwd"  , COSFile::getCurrentDir     ());

  prompt_type_    = CWSH_PROMPT_TYPE_NORMAL;
  prompt_command_ = "";

  if (interactive_)
    defineVariable("prompt", "> ");

  defineVariable("history", 20);

  defineVariable("status", 0);

  if (CEnvInst.exists("TERM")) {
    string term_env = CEnvInst.get("TERM");

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

  if (! fast_startup_)
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
    input_->execute(input_file_);

  CwshMgrInst.term(this, 0);
}

void
Cwsh::
processLine(const string &line)
{
  processInputLine(line);
}

void
Cwsh::
startup()
{
  string home = COSUser::getUserHome();

  CDir dir(home);

  dir.enter();

  if (login_shell_) {
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
  if (getNumActiveProcesses() > 0 && term_tries_ <= 0) {
    std::cerr << "There are suspended jobs." << std::endl;
    ++term_tries_;
    return;
  }

  // TODO: send all foreground and background process groups a SIGHUP, followed
  // by a SIGCONT signal (optional ?) if stopped

  if (login_shell_) {
    string home = COSUser::getUserHome();

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
changeDir(const string &dirname)
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

void
Cwsh::
defineFunction(const CwshFunctionName &name, const CwshLineArray &lines)
{
  function_mgr_->define(name, lines);
}

void
Cwsh::
undefineFunction(const CwshFunctionName &name)
{
  function_mgr_->undefine(name);
}

CwshFunction *
Cwsh::
lookupFunction(const CwshFunctionName &name)
{
  return function_mgr_->lookup(name);
}

void
Cwsh::
listAllFunctions()
{
  function_mgr_->listAll();
}

//---------------

void
Cwsh::
defineVariable(const CwshVariableName &name)
{
  if (variable_mgr_ != NULL)
    variable_mgr_->define(name);
}

void
Cwsh::
defineVariable(const CwshVariableName &name,
               const CwshVariableValue &value)
{
  if (variable_mgr_ != NULL)
    variable_mgr_->define(name, value);
}

void
Cwsh::
defineVariable(const CwshVariableName &name, int value)
{
  if (variable_mgr_ != NULL)
    variable_mgr_->define(name, value);
}

void
Cwsh::
defineVariable(const CwshVariableName &name,
               const CwshVariableValueArray &values)
{
  if (variable_mgr_ != NULL)
    variable_mgr_->define(name, values);
}

void
Cwsh::
defineVariable(const CwshVariableName &name,
               const char **values, int num_values)
{
  if (variable_mgr_ != NULL)
    variable_mgr_->define(name, values, num_values);
}

void
Cwsh::
undefineVariable(const CwshVariableName &name)
{
  if (variable_mgr_ != NULL)
    variable_mgr_->undefine(name);
}

CwshVariable *
Cwsh::
lookupVariable(const CwshVariableName &name) const
{
  if (variable_mgr_ != NULL)
    return variable_mgr_->lookup(name);
  else
    return NULL;
}

CwshVariableList::iterator
Cwsh::
variablesBegin()
{
  assert(variable_mgr_ != NULL);

  return variable_mgr_->variablesBegin();
}

CwshVariableList::iterator
Cwsh::
variablesEnd()
{
  assert(variable_mgr_ != NULL);

  return variable_mgr_->variablesEnd();
}

void
Cwsh::
listVariables() const
{
  if (variable_mgr_ != NULL)
    variable_mgr_->listVariables();
}

void
Cwsh::
saveVariables()
{
  if (variable_mgr_ != NULL)
    variable_mgr_->save();
}

void
Cwsh::
restoreVariables()
{
  if (variable_mgr_ != NULL)
    variable_mgr_->restore();
}

bool
Cwsh::
isEnvironmentVariableLower(const string &name)
{
  if (variable_mgr_ != NULL)
    return variable_mgr_->isEnvironmentVariableLower(name);
  else
    return false;
}

bool
Cwsh::
isEnvironmentVariableUpper(const string &name)
{
  if (variable_mgr_ != NULL)
    return variable_mgr_->isEnvironmentVariableUpper(name);
  else
    return false;
}

void
Cwsh::
updateEnvironmentVariable(CwshVariable *variable)
{
  if (variable_mgr_ != NULL)
    variable_mgr_->updateEnvironmentVariable(variable);
}

//---------------

CwshProcess *
Cwsh::
addProcess(CwshCommandData *command)
{
  return process_mgr_->add(command);
}

void
Cwsh::
removeProcess(CwshProcess *process)
{
  process_mgr_->remove(process);
}

void
Cwsh::
killProcess(int pid, int signal)
{
  process_mgr_->kill(pid, signal);
}

int
Cwsh::
getNumActiveProcesses()
{
  if (! process_mgr_) return 0;

  return process_mgr_->getNumActive();
}

void
Cwsh::
displayActiveProcesses(bool list_pids)
{
  process_mgr_->displayActive(list_pids);
}

void
Cwsh::
displayExitedProcesses()
{
  process_mgr_->displayExited();
}

void
Cwsh::
waitActiveProcesses()
{
  process_mgr_->waitActive();
}

int
Cwsh::
stringToProcessId(const string &str)
{
  return process_mgr_->stringToPid(str);
}

CwshProcess *
Cwsh::
getActiveProcess(const string &str)
{
  return process_mgr_->getActiveProcess(str);
}

CwshProcess *
Cwsh::
getCurrentActiveProcess()
{
  return process_mgr_->getCurrentActiveProcess();
}

CwshProcess *
Cwsh::
lookupProcess(pid_t pid)
{
  return process_mgr_->lookupProcess(pid);
}

//---------------

void
Cwsh::
saveState()
{
  state_mgr_->save(this);
}

void
Cwsh::
restoreState()
{
  state_mgr_->restore();
}

//---------------

void
Cwsh::
startBlock(CwshBlockType type, const CwshLineArray &lines)
{
  block_mgr_->startBlock(type, lines);
}

void
Cwsh::
endBlock()
{
  block_mgr_->endBlock();
}

bool
Cwsh::
inBlock() const
{
  return block_mgr_->inBlock();
}

bool
Cwsh::
blockEof() const
{
  return block_mgr_->eof();
}

CwshLine
Cwsh::
blockReadLine() const
{
  return block_mgr_->readLine();
}

CwshBlock *
Cwsh::
findBlock(CwshBlockType type)
{
  return block_mgr_->find(type);
}

void
Cwsh::
gotoBlockLabel(const string &label)
{
  block_mgr_->gotoLabel(label);
}

bool
Cwsh::
isBlockBreak() const
{
  return block_mgr_->isBreak();
}

bool
Cwsh::
isBlockBreakSwitch() const
{
  return block_mgr_->isBreakSwitch();
}

bool
Cwsh::
isBlockContinue() const
{
  return block_mgr_->isContinue();
}

bool
Cwsh::
isBlockReturn() const
{
  return block_mgr_->isReturn();
}

int
Cwsh::
getBlockGotoDepth() const
{
  return block_mgr_->getGotoDepth();
}

void
Cwsh::
setBlockBreak(bool flag)
{
  return block_mgr_->setBreak(flag);
}

void
Cwsh::
setBlockBreakSwitch(bool flag)
{
  return block_mgr_->setBreakSwitch(flag);
}

void
Cwsh::
setBlockContinue(bool flag)
{
  return block_mgr_->setContinue(flag);
}

void
Cwsh::
setBlockReturn(bool flag)
{
  return block_mgr_->setReturn(flag);
}

//---------------

void
Cwsh::
defineAlias(const CwshAliasName &name, const CwshAliasValue &value)
{
  alias_mgr_->define(name, value);
}

void
Cwsh::
undefineAlias(const CwshAliasName &name)
{
  alias_mgr_->undefine(name);
}

CwshAlias *
Cwsh::
lookupAlias(const CwshAliasName &name) const
{
  return alias_mgr_->lookup(name);
}

bool
Cwsh::
substituteAlias(CwshCmd *cmd, CwshCmdArray &cmds) const
{
  return alias_mgr_->substitute(cmd, cmds);
}

void
Cwsh::
displayAlias() const
{
  alias_mgr_->display();
}

//---------------

void
Cwsh::
defineAutoExec(const CwshAutoExecName &name, const CwshAutoExecValue &value)
{
  auto_exec_mgr_->define(name, value);
}

void
Cwsh::
undefineAutoExec(const CwshAutoExecName &name)
{
  auto_exec_mgr_->undefine(name);
}

CwshAutoExec *
Cwsh::
lookupAutoExec(const CwshAutoExecName &name) const
{
  return auto_exec_mgr_->lookup(name);
}

void
Cwsh::
displayAutoExec() const
{
  auto_exec_mgr_->display();
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
findHistoryCommandStart(const string &text, int &command_num)
{
  return history_->findCommandStart(text, command_num);
}

bool
Cwsh::
findHistoryCommandIn(const string &text, int &command_num)
{
  return history_->findCommandIn(text, command_num);
}

bool
Cwsh::
findHistoryCommandArg(const string &text, int &command_num, int &arg_num)
{
  return history_->findCommandArg(text, command_num, arg_num);
}

string
Cwsh::
getHistoryCommand(int num)
{
  return history_->getCommand(num);
}

string
Cwsh::
getHistoryCommandArg(int num, int arg_num)
{
  return history_->getCommandArg(num, arg_num);
}

void
Cwsh::
addHistoryFile(const string &filename)
{
  history_->addFile(filename);
}

void
Cwsh::
addHistoryCommand(const string &text)
{
  history_->addCommand(text);
}

void
Cwsh::
setHistoryCurrent(const string &text)
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

string
Cwsh::
getPrevHistoryCommand()
{
  return history_->getPrevCommand();
}

string
Cwsh::
getNextHistoryCommand()
{
  return history_->getNextCommand();
}

//------------

CwshShellCommand *
Cwsh::
lookupShellCommand(const string &name) const
{
  return shell_cmd_mgr_->lookup(name);
}

//------------

void
Cwsh::
executeInput(const string &filename)
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

string
Cwsh::
getInputPrompt()
{
  if (getSilentMode())
    return "";

  return input_->getPrompt();
}

string
Cwsh::
processInputExprLine(const CwshLine &line)
{
  return input_->processExprLine(line);
}

//------------

string
Cwsh::
readLine()
{
  //struct termios t;

  //if (getSilentMode()) COSPty::set_raw(0, &t);

  string str = read_line_->readLine();

  //if (getSilentMode()) COSPty::reset_raw(0, &t);

  return str;
}

void
Cwsh::
beep()
{
  read_line_->beep();
}

void
Cwsh::
readInterrupt()
{
  read_line_->interrupt();
}

//------------

void
Cwsh::
pushDirStack()
{
  dir_stack_->push();
}

void
Cwsh::
pushDirStack(const string &dirname)
{
  dir_stack_->push(dirname);
}

string
Cwsh::
popDirStack()
{
  return dir_stack_->pop();
}

string
Cwsh::
popDirStack(int pos)
{
  return dir_stack_->pop(pos);
}

int
Cwsh::
sizeDirStack()
{
  return dir_stack_->size();
}

void
Cwsh::
printDirStack(bool /*expand_home*/)
{
  dir_stack_->print();
}

//------------

void
Cwsh::
addFilePath(const string &filename, const string &path)
{
  hash_->addFilePath(filename, path);
}

string
Cwsh::
getFilePath(const string &filename)
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
limitResource(const string &name, const string &value, bool hard)
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
unlimitResource(const string &name)
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
printResource(const string &name, bool hard)
{
  resource_->print(name, hard);
}

void
Cwsh::
readTimeout()
{
#if 0
  string line = read_line_->getBuffer();

  line = colorLine(line);

  read_line_->setBuffer(line);
#endif

  if (server_ != NULL)
    server_->processMessage();
}

string
Cwsh::
colorLine(const string &line)
{
  return "[1m" + line + "[0m";
}

string
Cwsh::
getAliasesMsg() const
{
  return alias_mgr_->getAliasesMsg();
}

string
Cwsh::
getHistoryMsg() const
{
  return history_->getHistoryMsg();
}
