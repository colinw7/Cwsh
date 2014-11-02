#include <CwshI.h>
#include <COSProcess.h>
#include <COSSignal.h>
#include <CFileMatch.h>
#include <cerrno>

#include <sys/resource.h>
#include <sys/times.h>

#define CWSH_SHELL_CMD_DATA1(n,p) \
  { n , NULL, &CwshShellCommandMgr::p, CWSH_SHELL_COMMAND_FLAGS_NONE }
#define CWSH_SHELL_CMD_DATA2(ns,ne,p) \
  { ns, ne, &CwshShellCommandMgr::p, CWSH_SHELL_COMMAND_FLAGS_NONE }
#define CWSH_SHELL_CMD_DATA1_NW(n,p) \
  { n , NULL, &CwshShellCommandMgr::p, CWSH_SHELL_COMMAND_FLAGS_NO_WILDCARDS }
#define CWSH_SHELL_CMD_DATA2_NE(ns,ne,p) \
  { ns, ne, &CwshShellCommandMgr::p, CWSH_SHELL_COMMAND_FLAGS_NO_EXPAND }

CwshShellCommandData
CwshShellCommandMgr::
commands_data_[] = {
 CWSH_SHELL_CMD_DATA2("foreach", "end"    , foreachCmd),
 CWSH_SHELL_CMD_DATA2("func"   , "endfunc", funcCmd   ),
 CWSH_SHELL_CMD_DATA2("if"     , "endif"  , ifCmd     ),
 CWSH_SHELL_CMD_DATA2("switch" , "endsw"  , switchCmd ),

 CWSH_SHELL_CMD_DATA2_NE("while"  , "end"    , whileCmd  ),

 CWSH_SHELL_CMD_DATA1(":"       , colonCmd   ),
 CWSH_SHELL_CMD_DATA1("bg"      , bgCmd      ),
 CWSH_SHELL_CMD_DATA1("break"   , breakCmd   ),
 CWSH_SHELL_CMD_DATA1("breaksw" , breakswCmd ),
 CWSH_SHELL_CMD_DATA1("case"    , caseCmd    ),
 CWSH_SHELL_CMD_DATA1("cd"      , cdCmd      ),
 CWSH_SHELL_CMD_DATA1("chdir"   , cdCmd      ),
 CWSH_SHELL_CMD_DATA1("continue", continueCmd),
 CWSH_SHELL_CMD_DATA1("default" , defaultCmd ),
 CWSH_SHELL_CMD_DATA1("dirs"    , dirsCmd    ),
 CWSH_SHELL_CMD_DATA1("echo"    , echoCmd    ),
 CWSH_SHELL_CMD_DATA1("else"    , elseCmd    ),
 CWSH_SHELL_CMD_DATA1("end"     , endCmd     ),
 CWSH_SHELL_CMD_DATA1("endfunc" , endfuncCmd ),
 CWSH_SHELL_CMD_DATA1("endif"   , endifCmd   ),
 CWSH_SHELL_CMD_DATA1("endsw"   , endswCmd   ),
 CWSH_SHELL_CMD_DATA1("eval"    , evalCmd    ),
 CWSH_SHELL_CMD_DATA1("exec"    , execCmd    ),
 CWSH_SHELL_CMD_DATA1("expr"    , exprCmd    ),
 CWSH_SHELL_CMD_DATA1("exit"    , exitCmd    ),
 CWSH_SHELL_CMD_DATA1("fg"      , fgCmd      ),
 CWSH_SHELL_CMD_DATA1("glob"    , globCmd    ),
 CWSH_SHELL_CMD_DATA1("goto"    , gotoCmd    ),
 CWSH_SHELL_CMD_DATA1("hashstat", hashstatCmd),
 CWSH_SHELL_CMD_DATA1("help"    , helpCmd    ),
 CWSH_SHELL_CMD_DATA1("history" , historyCmd ),
 CWSH_SHELL_CMD_DATA1("jobs"    , jobsCmd    ),
 CWSH_SHELL_CMD_DATA1("kill"    , killCmd    ),
 CWSH_SHELL_CMD_DATA1("limit"   , limitCmd   ),
 CWSH_SHELL_CMD_DATA1("login"   , badCmd     ),
 CWSH_SHELL_CMD_DATA1("logout"  , badCmd     ),
 CWSH_SHELL_CMD_DATA1("nice"    , niceCmd    ),
 CWSH_SHELL_CMD_DATA1("nohup"   , nohupCmd   ),
 CWSH_SHELL_CMD_DATA1("notify"  , notifyCmd  ),
 CWSH_SHELL_CMD_DATA1("onintr"  , onintrCmd  ),
 CWSH_SHELL_CMD_DATA1("popd"    , popdCmd    ),
 CWSH_SHELL_CMD_DATA1("printenv", printenvCmd),
 CWSH_SHELL_CMD_DATA1("pushd"   , pushdCmd   ),
 CWSH_SHELL_CMD_DATA1("rehash"  , rehashCmd  ),
 CWSH_SHELL_CMD_DATA1("repeat"  , repeatCmd  ),
 CWSH_SHELL_CMD_DATA1("return"  , returnCmd  ),
 CWSH_SHELL_CMD_DATA1("setenv"  , setenvCmd  ),
 CWSH_SHELL_CMD_DATA1("shift"   , shiftCmd   ),
 CWSH_SHELL_CMD_DATA1("source"  , sourceCmd  ),
 CWSH_SHELL_CMD_DATA1("stop"    , stopCmd    ),
 CWSH_SHELL_CMD_DATA1("suspend" , suspendCmd ),
 CWSH_SHELL_CMD_DATA1("time"    , timeCmd    ),
 CWSH_SHELL_CMD_DATA1("umask"   , umaskCmd   ),
 CWSH_SHELL_CMD_DATA1("unhash"  , unhashCmd  ),
 CWSH_SHELL_CMD_DATA1("unlimit" , unlimitCmd ),
 CWSH_SHELL_CMD_DATA1("unset"   , unsetCmd   ),
 CWSH_SHELL_CMD_DATA1("unsetenv", unsetenvCmd),
 CWSH_SHELL_CMD_DATA1("wait"    , waitCmd    ),
 CWSH_SHELL_CMD_DATA1("which"   , whichCmd   ),

 CWSH_SHELL_CMD_DATA1_NW("alias"   , aliasCmd   ),
 CWSH_SHELL_CMD_DATA1_NW("unalias" , unaliasCmd ),
 CWSH_SHELL_CMD_DATA1_NW("set"     , setCmd     ),
 CWSH_SHELL_CMD_DATA1_NW("complete", completeCmd),
 CWSH_SHELL_CMD_DATA1_NW("autoexec", autoExecCmd),
 CWSH_SHELL_CMD_DATA1_NW("@"       , atCmd      ),
};

CwshShellCommandMgr::
CwshShellCommandMgr(Cwsh *cwsh) :
 cwsh_(cwsh)
{
  uint num_commands = sizeof(commands_data_)/sizeof(commands_data_[0]);

  commands_.resize(num_commands);

  for (uint i = 0; i < num_commands; ++i)
    commands_[i] = new CwshShellCommand(cwsh_, &commands_data_[i]);
}

CwshShellCommandMgr::
~CwshShellCommandMgr()
{
  std::for_each(commands_.begin(), commands_.end(), CDeletePointer());
}

CwshShellCommand *
CwshShellCommandMgr::
lookup(const string &name) const
{
  if (name.size() > 0 && name[0] == '\\')
    return lookup(name.substr(1));

  uint num_commands = commands_.size();

  for (uint i = 0; i < num_commands; i++)
    if (commands_[i]->getName() == name)
      return commands_[i];

  return NULL;
}

void
CwshShellCommandMgr::
runProc(const CwshArgArray &args, CCommand::CallbackData data)
{
  CwshShellCommand *shell_command = (CwshShellCommand *) data;

  if (shell_command->getCwsh()->getDebug()) {
    std::cerr << shell_command->getName();

    int num_args = args.size();

    for (int i = 0; i < num_args; i++)
      std::cerr << " " << args[i];

    std::cerr << std::endl;
  }

  (shell_command->getProc())(shell_command->getCwsh(), args);
}

void
CwshShellCommandMgr::
colonCmd(Cwsh *, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    return;
  }
}

void
CwshShellCommandMgr::
aliasCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "alias               ; list all aliases" << std::endl;
    std::cout << "alias <name>        ; list named alias" << std::endl;
    std::cout << "alias <name> <args> ; define alias to args" << std::endl;
    return;
  }

  int num_args = args.size();

  if      (num_args == 0)
    cwsh->displayAlias();
  else if (num_args == 1) {
    CwshAlias *alias = cwsh->lookupAlias(args[0]);

    if (alias != NULL)
      std::cout << alias->getValue() << std::endl;
  }
  else {
    string cmd = CStrUtil::toString(args, 1);

    cwsh->defineAlias(args[0], cmd);
  }
}

void
CwshShellCommandMgr::
autoExecCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "autoexec                 ; list all auto exec rules" << std::endl;
    std::cout << "autoexec <suffix>        ; list named auto exec" << std::endl;
    std::cout << "autoexec <suffix> <args> ; define auto exec for suffix to args" << std::endl;
    return;
  }

  int num_args = args.size();

  if      (num_args == 0)
    cwsh->displayAutoExec();
  else if (num_args == 1) {
    CwshAutoExec *autoExec = cwsh->lookupAutoExec(args[0]);

    if (autoExec != NULL)
      std::cout << autoExec->getValue() << std::endl;
  }
  else {
    string cmd = CStrUtil::toString(args, 1);

    cwsh->defineAutoExec(args[0], cmd);
  }
}

void
CwshShellCommandMgr::
bgCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "bg           ; move current process to backgroud" << std::endl;
    std::cout << "bg <job> ... ; move specified jobs to background" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args > 0) {
    for (int i = 0; i < num_args; i++) {
      CwshProcess *process = cwsh->getActiveProcess(args[i]);

      if (process == NULL)
        CWSH_THROW("No such job.");

      std::cout << "[" << process->getNum() << "]    ";

      process->print();

      std::cout << " &" << std::endl;

      process->resume();
    }
  }
  else {
    CwshProcess *process = cwsh->getCurrentActiveProcess();

    if (process == NULL)
      CWSH_THROW("No current job.");

    std::cout << "[" << process->getNum() << "]    ";

    process->print();

    std::cout << " &" << std::endl;

    process->resume();
  }
}

void
CwshShellCommandMgr::
breakCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "break ; break out of while/foreach" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args > 0)
    CWSH_THROW("Too many arguments.");

  CwshBlock *block = cwsh->findBlock(CWSH_BLOCK_TYPE_WHILE);

  if (block == NULL)
    block = cwsh->findBlock(CWSH_BLOCK_TYPE_FOREACH);

  if (block == NULL)
    CWSH_THROW("Not in while/foreach.");

  cwsh->setBlockBreak(true);
}

void
CwshShellCommandMgr::
breakswCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "breaksw ; break out of switch" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args > 0)
    CWSH_THROW("Too many arguments.");

  CwshBlock *block = cwsh->findBlock(CWSH_BLOCK_TYPE_SWITCH);

  if (block == NULL)
    CWSH_THROW("Not in switch.");

  cwsh->setBlockBreakSwitch(true);
}

void
CwshShellCommandMgr::
caseCmd(Cwsh *, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "case <expr> ; switch case statement" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args != 2)
    CWSH_THROW("Syntax Error.");

  if (args[1] != ":")
    CWSH_THROW("Syntax Error.");

  CWSH_THROW("Not in switch.");
}

void
CwshShellCommandMgr::
cdCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "cd           ; change to home directory" << std::endl;
    std::cout << "cd <dir>     ; change to specified directory" << std::endl;
    std::cout << "cd <dir> ... ; change to first valid directory" << std::endl;

    return;
  }

  int num_args = args.size();

  if (num_args == 0) {
    string dirname = COSUser::getUserHome();

    cwsh->changeDir(dirname);
  }
  else {
    for (int i = 0; i < num_args; ++i) {
      string dirname = CStrUtil::stripSpaces(args[i]);

      if (dirname == "")
        dirname = COSUser::getUserHome();

      if (i == num_args - 1)
        dirname = CwshDir::lookup(cwsh, dirname, true);
      else {
        dirname = CwshDir::lookup(cwsh, dirname, false);

        if (dirname == "") continue;
      }

      cwsh->changeDir(dirname);
    }
  }
}

void
CwshShellCommandMgr::
completeCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "complete [-all|-best|-command|-file|-var] <pattern> ... ; "
                 "complete pattern" << std::endl;
    return;
  }

  enum CwshCompleteShow {
    COMPLETE_SHOW_ALL,
    COMPLETE_SHOW_BEST
  };

  enum CwshCompleteType {
    COMPLETE_TYPE_COMMAND,
    COMPLETE_TYPE_FILE,
    COMPLETE_TYPE_VAR
  };

  vector<string> args1;

  CwshCompleteShow show = COMPLETE_SHOW_ALL;
  CwshCompleteType type = COMPLETE_TYPE_COMMAND;

  int num_args = args.size();

  for (int i = 0; i < num_args; i++) {
    if (args[i][0] == '-') {
      string name = args[i].substr(1);

      if      (name == "all")
        show = COMPLETE_SHOW_ALL;
      else if (name == "best")
        show = COMPLETE_SHOW_BEST;
      else if (name == "command")
        type = COMPLETE_TYPE_COMMAND;
      else if (name == "file")
        type = COMPLETE_TYPE_FILE;
      else if (name == "var")
        type = COMPLETE_TYPE_VAR;
      else
        CWSH_THROW("Invalid argument.");
    }
    else
      args1.push_back(args[i]);
  }

  int num_args1 = args1.size();

  if (num_args1 < 1)
    CWSH_THROW("Too few arguments.");

  if (num_args1 > 1)
    CWSH_THROW("Too many arguments.");

  if (show == COMPLETE_SHOW_ALL) {
    string pattern_str = args1[0] + "*";

    vector<string> names;

    if      (type == COMPLETE_TYPE_COMMAND) {
      CwshPattern pattern(cwsh, pattern_str);

      pattern.expandPath(names);
    }
    else if (type == COMPLETE_TYPE_FILE) {
      CFileMatch fileMatch;

      fileMatch.matchPattern(pattern_str, names);
    }
    else {
      CwshPattern pattern(cwsh, pattern_str);

      pattern.expandVar(names);
    }

    int num_names = names.size();

    for (int i = 0; i < num_names; i++)
      std::cout << names[i] << std::endl;
  }
  else {
    string word;

    CwshComplete complete(cwsh, args1[0]);

    bool flag;

    if      (type == COMPLETE_TYPE_COMMAND)
      flag = complete.completeCommand (word);
    else if (type == COMPLETE_TYPE_FILE)
      flag = complete.completeFile    (word);
    else
      flag = complete.completeVariable(word);

    if (! flag)
      cwsh->beep();

    std::cout << word << std::endl;
  }
}

void
CwshShellCommandMgr::
continueCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "continue ; continue to next iteration of while/foreach" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args > 0)
    CWSH_THROW("Too many arguments.");

  CwshBlock *block = cwsh->findBlock(CWSH_BLOCK_TYPE_WHILE);

  if (block == NULL)
    block = cwsh->findBlock(CWSH_BLOCK_TYPE_FOREACH);

  if (block == NULL)
    CWSH_THROW("Not in while/foreach.");

  cwsh->setBlockContinue(true);
}

void
CwshShellCommandMgr::
defaultCmd(Cwsh *, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "default ; switch default statement" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args != 1)
    CWSH_THROW("Syntax Error.");

  if (args[0] != ":")
    CWSH_THROW("Syntax Error.");

  CWSH_THROW("Not in switch.");
}

void
CwshShellCommandMgr::
dirsCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "dirs [-l] ; print directory stack" << std::endl;
    return;
  }

  int num_args = args.size();

  int i = 0;

  bool expand_home = false;

  if (num_args > 0 && args[i] == "-l") {
    expand_home = true;

    i++;
  }

  if (i > num_args)
    CWSH_THROW("Too many arguments.");

  cwsh->printDirStack(expand_home);
}

void
CwshShellCommandMgr::
echoCmd(Cwsh *, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "echo [-n] args ; print arguments" << std::endl;
    return;
  }

  int num_args = args.size();

  int i = 0;

  bool new_line = true;

  if (num_args > 0 && args[0] == "-n") {
    new_line = false;

    i++;
  }

  if (i < num_args) {
    std::cout << args[i++];

    for ( ; i < num_args; i++)
      std::cout << " " << args[i];
  }
  else
    new_line = false;

  if (new_line)
    std::cout << std::endl;

  std::cout.flush();
}

void
CwshShellCommandMgr::
elseCmd(Cwsh *, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "else ; else clause of if" << std::endl;
    return;
  }

  CWSH_THROW("Not in if.");
}

void
CwshShellCommandMgr::
endCmd(Cwsh *, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "end ; end of while/foreach" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args > 0)
    CWSH_THROW("Too many arguments.");

  CWSH_THROW("Not in while/foreach.");
}

void
CwshShellCommandMgr::
endfuncCmd(Cwsh *, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "endfunc ; end of func" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args > 0)
    CWSH_THROW("Too many arguments.");

  CWSH_THROW("Not in func.");
}

void
CwshShellCommandMgr::
endifCmd(Cwsh *, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "endif ; end of if" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args > 0)
    CWSH_THROW("Too many arguments.");

  CWSH_THROW("Not in if.");
}

void
CwshShellCommandMgr::
endswCmd(Cwsh *, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "endsw ; end of switch" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args > 0)
    CWSH_THROW("Too many arguments.");

  CWSH_THROW("Not in switch.");
}

void
CwshShellCommandMgr::
evalCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "eval <arg> ... ; evaluate arg as if entered as input" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args == 0)
    CWSH_THROW("Too few arguments.");

  string line = CStrUtil::toString(args, " ");

  cwsh->processInputLine(line);
}

void
CwshShellCommandMgr::
execCmd(Cwsh *, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "exec <arg> ... ; execute command to replace shell" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args == 0)
    CWSH_THROW("Too few arguments.");

  vector<char *> cargs;

  cargs.resize(num_args + 1);

  int i = 0;

  for ( ; i < num_args; i++)
    cargs[i] = (char *) args[i].c_str();
  cargs[i] = NULL;

  execvp(cargs[0], &cargs[0]);

  _exit(255);
}

void
CwshShellCommandMgr::
exitCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "exit            ; exit shell" << std::endl;
    std::cout << "exit <expr> ... ; exit shell with specified return value" << std::endl;
    return;
  }

  int num_args = args.size();

  int status = 1;

  if (num_args > 0) {
    string expr_str = CStrUtil::toString(args, " ");

    CwshExprEvaluate expr(cwsh, expr_str);

    status = expr.process();
  }
  else {
    CwshVariable *variable = cwsh->lookupVariable("status");

    if (variable != NULL && variable->getNumValues() == 1) {
      if (CStrUtil::isInteger(variable->getValue(0)))
        status = CStrUtil::toInteger(variable->getValue(0));
    }
  }

  cwsh->setExit(true, status);
}

void
CwshShellCommandMgr::
exprCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "expr <expr> ... ; evaluate expression" << std::endl;
    return;
  }

  string expr_str = CStrUtil::toString(args, " ");

  CwshExprEvaluate expr(cwsh, expr_str);

  std::cout << expr.process() << std::endl;
}

void
CwshShellCommandMgr::
fgCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "fg           ; move current process to foreground" << std::endl;
    std::cout << "fg <job> ... ; move specified jobs to foreground" << std::endl;
    return;
  }

  uint num_args = args.size();

  if (num_args > 0) {
    for (uint i = 0; i < num_args; ++i) {
      CwshProcess *process = cwsh->getActiveProcess(args[i]);

      if (process == NULL)
        CWSH_THROW("No such job.");

      process->print();

      std::cout << std::endl;

      process->resume();

      process->wait();
    }
  }
  else {
    CwshProcess *process = cwsh->getCurrentActiveProcess();

    if (process == NULL)
      CWSH_THROW("No current job.");

    process->print();

    std::cout << std::endl;

    process->resume();

    process->wait();
  }
}

void
CwshShellCommandMgr::
foreachCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "foreach <arg> (<expr>) ; "
                 "loop for each value of <expr> setting <arg> to value" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args < 3)
    CWSH_THROW("Too few arguments.");

  string varname = args[0];

  if (args[1] != "(" || args[num_args - 1] != ")")
    CWSH_THROW("Words not parenthesized.");

  vector<string> values;

  for (int i = 2; i < num_args - 1; i++)
    values.push_back(args[i]);

  CwshLineArray lines;

  CwshShellCommand *command = cwsh->lookupShellCommand("foreach");

  cwsh->getInputBlock(command, lines);

  int num_values = values.size();

  for (int i = 0; i < num_values; i++) {
    cwsh->defineVariable(varname, values[i]);

    cwsh->startBlock(CWSH_BLOCK_TYPE_FOREACH, lines);

    while (! cwsh->inputEof()) {
      string line = cwsh->getInputLine();

      cwsh->processInputLine(line);

      if (cwsh->isBlockBreak   () ||
          cwsh->isBlockContinue() ||
          cwsh->isBlockReturn  ())
        break;

      if (cwsh->getBlockGotoDepth() > 0) {
        cwsh->setBlockBreak(true);
        break;
      }
    }

    cwsh->endBlock();

    if (cwsh->isBlockContinue() ||
        cwsh->isBlockReturn  ())
      break;
  }

  cwsh->setBlockContinue(false);
  cwsh->setBlockBreak(false);
}

void
CwshShellCommandMgr::
funcCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "func <name> <args> ; define function <name>" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args == 0) {
    cwsh->listAllFunctions();

    return;
  }

  if (num_args != 3)
    CWSH_THROW("Too few arguments.");

  if (args[1] != "(" || args[2] != ")")
    CWSH_THROW("Syntax Error.");

  CwshShellCommand *command = cwsh->lookupShellCommand("func");

  CwshLineArray lines;

  cwsh->getInputBlock(command, lines);

  cwsh->defineFunction(args[0], lines);
}

void
CwshShellCommandMgr::
globCmd(Cwsh *, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "glob <arg> ... ; expand supplied arguments" << std::endl;
    return;
  }

  int num_args = args.size();

  string str;

  for (int i = 0; i < num_args; i++)
    str += args[i];

  std::cout << str;
}

void
CwshShellCommandMgr::
gotoCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "goto <label> ; goto specified label" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args == 0)
    CWSH_THROW("Too few arguments.");

  if (num_args > 1)
    CWSH_THROW("Too many arguments.");

  cwsh->gotoBlockLabel(args[0]);
}

void
CwshShellCommandMgr::
hashstatCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "hashstat ; display command hashing statistics" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args != 0)
    CWSH_THROW("Too many arguments.");

  cwsh->printFilePathStats();
}

void
CwshShellCommandMgr::
helpCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "help       ; display commands" << std::endl;
    std::cout << "help <cmd> ; display help for specified command" << std::endl;
    return;
  }

  bool show_all = false;

  vector<string> cmds;

  uint num_args = args.size();

  for (uint i = 0; i < num_args; ++i) {
    if (args[i][0] == '-') {
      string name = args[i].substr(1);

      if      (name == "a")
        show_all = true;
      else
        CWSH_THROW("Invalid argument.");
    }
    else
      cmds.push_back(args[i]);
  }

  CwshShellCommandMgr *mgr = cwsh->getShellCommandMgr();

  uint num_cmds = cmds.size();

  if (num_cmds == 0) {
    set<string> cmds;

    uint num_commands = mgr->commands_.size();

    uint i = 0;

    for ( ; i < num_commands; ++i)
      cmds.insert(mgr->commands_[i]->getName());

    set<string>::const_iterator p1, p2;

    for (i = 0, p1 = cmds.begin(), p2 = cmds.end(); p1 != p2; ++i, ++p1) {
      if (show_all) {
        CwshShellCommand *command = mgr->lookup(*p1);

        CwshArgArray args;

        args.push_back("--help");

        (command->getProc())(cwsh, args);
      }
      else {
        if (i > 0) std::cout << " ";

        std::cout << *p1;
      }
    }

    if (! show_all)
      std::cout << std::endl;
  }
  else {
    for (uint i = 0; i < num_cmds; ++i) {
      CwshShellCommand *command = mgr->lookup(args[i]);

      if (command != NULL) {
        CwshArgArray args;

        args.push_back("--help");

        (command->getProc())(cwsh, args);
      }
      else
        CWSH_THROW("Unknown command " + args[0]);
    }
  }
}

void
CwshShellCommandMgr::
historyCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "history [-h|-r] ; display history" << std::endl;
    std::cout << "history <num>   ; display numbered history event" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args > 2)
    CWSH_THROW("Too many arguments.");

  bool show_numbers = true;
  bool show_time    = true;
  bool reverse      = false;
  int  num          = -1;

  for (int i = 0; i < num_args; i++) {
    if      (args[i] == "-h") {
      show_numbers = false;
      show_time    = false;
    }
    else if (args[i] == "-r")
      reverse = true;
    else {
      if (! CStrUtil::isInteger(args[i]))
        CWSH_THROW("Badly formed number.");

      num = CStrUtil::toInteger(args[i]);
    }
  }

  cwsh->displayHistory(num, show_numbers, show_time, reverse);
}

void
CwshShellCommandMgr::
ifCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "if (<expr>) then ; start of if statement" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args == 0)
    CWSH_THROW("Too few arguments.");

  string str = CStrUtil::toString(args, " ");

  uint i = 0;

  CwshExprParse parse(cwsh);

  string expr_str = parse.parse(str, &i);

  CStrUtil::skipSpace(str, &i);

  uint j = i;

  CStrUtil::skipNonSpace(str, &j);

  string word = str.substr(i, j - i);

  if (word == "then") {
    i = j;

    CStrUtil::skipSpace(str, &i);

    uint len = str.size();

    if (i < len)
      CWSH_THROW("Improper then.");

    CwshShellCommand *command = cwsh->lookupShellCommand("if");

    CwshLineArray lines;

    cwsh->getInputBlock(command, lines);

    if (cwsh->getDebug()) {
      std::cerr << "if ( " << expr_str << " ) then" << std::endl;

      uint num_lines = lines.size();

      for (i = 0; i < num_lines; i++)
        std::cerr << lines[i] << std::endl;

      std::cerr << "endif" << std::endl;
    }

    CwshExprEvaluate expr(cwsh, expr_str);

    int processing = expr.process();

    bool if_processed = processing;

    cwsh->startBlock(CWSH_BLOCK_TYPE_IF, lines);

    while (! cwsh->inputEof()) {
      string line = cwsh->getInputLine();

      vector<string> words;

      CwshString::addWords(line, words);

      if (words.size() > 0 && words[0] == "else") {
        if (words.size() > 1 && words[1] == "if") {
          string str = CStrUtil::toString(words, 2, -1);

          uint i = 0;

          CwshExprParse parse(cwsh);

          string expr_str = parse.parse(str, &i);

          CStrUtil::skipSpace(str, &i);

          uint j = i;

          CStrUtil::skipNonSpace(str, &j);

          string word = str.substr(i, j - i);

          if (word == "then") {
            i = j;

            CStrUtil::skipSpace(str, &i);

            uint len = str.size();

            if (i < len)
              CWSH_THROW("Improper then.");

            CwshExprEvaluate expr(cwsh, expr_str);

            int processing1 = expr.process();

            if (! if_processed)
              processing = processing1;

            if (processing)
              if_processed = true;
          }
          else {
            string line = str.substr(i);

            CwshExprEvaluate expr(cwsh, expr_str);

            int processing1 = expr.process();

            if (! if_processed)
              processing = processing1;
            else
              processing = false;

            if (processing)
              if_processed = true;

            if (processing) {
              cwsh->processInputLine(line);

              processing = false;
            }
          }
        }
        else {
          processing = ! if_processed;

          if (processing)
            if_processed = true;

          if (words.size() > 1) {
            string line = CStrUtil::toString(words, 1, -1);

            cwsh->processInputLine(line);

            processing = false;
          }
        }
      }
      else {
        if (processing)
          cwsh->processInputLine(line);
        else
          cwsh->skipInputBlock(line);
      }

      if (cwsh->isBlockBreak   () ||
          cwsh->isBlockContinue() ||
          cwsh->isBlockReturn  ())
        break;

      if (cwsh->getBlockGotoDepth() > 0)
        break;
    }

    cwsh->endBlock();
  }
  else {
    string line = str.substr(i);

    if (cwsh->getDebug())
      std::cerr << "if ( " << expr_str << " ) " << line << std::endl;

    CwshExprEvaluate expr(cwsh, expr_str);

    int processing = expr.process();

    if (processing)
      cwsh->processInputLine(line);
  }
}

void
CwshShellCommandMgr::
jobsCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "jobs [-l] ; display jobs" << std::endl;
    return;
  }

  int num_args = args.size();

  bool list_pids = false;

  for (int i = 0; i < num_args; i++) {
    if      (args[i] == "-l")
      list_pids = true;
    else
      CWSH_THROW("Usage: jobs [ -l ].");
  }

  cwsh->displayActiveProcesses(list_pids);
}

void
CwshShellCommandMgr::
killCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "kill -l                       ; list signals" << std::endl;
    std::cout << "kill [-<num>|-<signal>] <pid> ; kill process" << std::endl;
    return;
  }

  int num_args = args.size();

  bool list_signals = false;

  int signal_num = -1;

  int i = 0;

  for ( ; i < num_args; i++) {
    if      (args[i] == "-l")
      list_signals = true;
    else if (args[i].size() > 0 && args[i][0] == '-') {
      string arg = args[i].substr(1);

      if (args[i].size() > 1 && isdigit(args[i][1])) {
        if (! CStrUtil::isInteger(arg))
          CWSH_THROW("Badly formed number.");

        signal_num = CStrUtil::toInteger(arg);

        CwshSignal *signal = CwshSignal::lookup(signal_num);

        if (signal == NULL)
          CWSH_THROW("Bad signal number.");
      }
      else {
        CwshSignal *signal = CwshSignal::lookup(arg);

        if (signal == NULL)
          CWSH_THROW("Unknown signal name.");

        signal_num = signal->getNum();
      }
    }
    else
      break;
  }

  if (list_signals) {
    int num_signals = CwshSignal::getNumSignals();

    for (int j = 0; j < num_signals; j++) {
      CwshSignal *signal = CwshSignal::getSignal(j);

      std::cout << signal->getName() << " ";
    }

    std::cout << std::endl;

    return;
  }

  if (signal_num == -1)
    signal_num = SIGTERM;

  if (i >= num_args)
    CWSH_THROW("Too few arguments.");

  vector<int> pids;

  for ( ; i < num_args; i++) {
    int pid;

    if (args[i].size() > 0 && args[i][0] == '%')
      pid = cwsh->stringToProcessId(args[i]);
    else {
      if (! CStrUtil::isInteger(args[i]))
        CWSH_THROW("Arguments should be jobs or process id's.");

      pid = CStrUtil::toInteger(args[i]);
    }

    pids.push_back(pid);
  }

  uint num_pids = pids.size();

  for (uint i = 0; i < num_pids; ++i)
    cwsh->killProcess(pids[i], signal_num);
}

void
CwshShellCommandMgr::
limitCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "limit [-h] <name> <value> ; set named limit ti value" << std::endl;
    return;
  }

  int num_args = args.size();

  bool   hard  = false;
  string name  = "";
  string value = "";

  for (int i = 0; i < num_args; i++) {
    if      (args[i] == "-h")
      hard = true;
    else if (name == "")
      name = args[i];
    else if (value == "")
      value = args[i];
    else
      CWSH_THROW("Too many arguments.");
  }

  if (name == "") {
    cwsh->printAllResources(hard);

    return;
  }

  if (value == "") {
    cwsh->printResource(name, hard);

    return;
  }

  cwsh->limitResource(name, value, hard);
}

void
CwshShellCommandMgr::
niceCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "nice                       ; "
                 "get priority of current process" << std::endl;
    std::cout << "nice [+<num>|-<num>        ; "
                 "increase/decrease priority of current process" << std::endl;
    std::cout << "nice [+<num>|-<num>] <pid> ; "
                 "run command as specified priority" << std::endl;
    return;
  }

  int num_args = args.size();

  int pid = COSProcess::getProcessId();

  errno = 0;

  int priority = getpriority(PRIO_PROCESS, pid);

  if (priority == -1 && errno != 0)
    CWSH_THROW("getpriority failed.");

  if (num_args == 0) {
    std::cout << "Current Priority " << priority << std::endl;

    return;
  }

  int dpriority = 0;

  int i = 0;

  if (num_args > 0 && (args[i][0] == '+' || args[i][0] == '-')) {
    string istr = args[i].substr(1);

    if (! CStrUtil::isInteger(istr))
      CWSH_THROW("Invalid argument.");

    dpriority = CStrUtil::toInteger(istr);

    if (args[i][0] == '-')
      priority -= dpriority;
    else
      priority += dpriority;

    if (priority < -20 || priority > 20)
      CWSH_THROW("Invalid priority.");

    i++;
  }

  if (i < num_args) {
    CWSH_THROW("Not implemented.");

    int error = nice(dpriority);

    if (error < 0)
      CWSH_THROW("nice failed.");

    string command = CStrUtil::toString(args, i);

    cwsh->processInputLine(command);
  }
  else {
    int error = nice(dpriority);

    if (error < 0)
      CWSH_THROW("nice failed.");
  }
}

void
CwshShellCommandMgr::
nohupCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "nohup           ; set no hangup" << std::endl;
    std::cout << "nohup <command> ; run command with no hangup" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args == 0)
    CwshSignal::nohup();
  else {
    // TODO: start command with SIGHUP disabled

    CWSH_THROW("Not implemented.");

    string command = CStrUtil::toString(args, " ");

    cwsh->processInputLine(command);
  }
}

void
CwshShellCommandMgr::
notifyCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "notify       ; notify current processs" << std::endl;
    std::cout << "notify <pid> ; notify processs" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args > 0) {
    for (int i = 0; i < num_args; i++) {
      CwshProcess *process = cwsh->getActiveProcess(args[i]);

      if (process == NULL)
        CWSH_THROW("No such job.");

      process->setNotify(true);
    }
  }
  else {
    CwshProcess *process = cwsh->getCurrentActiveProcess();

    if (process == NULL)
      CWSH_THROW("No current job.");

    process->setNotify(true);
  }
}

void
CwshShellCommandMgr::
onintrCmd(Cwsh *, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "onintr         ; reset interrupts" << std::endl;
    std::cout << "onintr -       ; ignore interrupts" << std::endl;
    std::cout << "onintr <label> ; goto label on interrupt" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args > 1)
    CWSH_THROW("Too many arguments.");

  if (num_args == 0)
    CwshSignal::interruptReset();
  else {
    if (args[0] == "-")
      CwshSignal::interruptIgnore();
    else
      CwshSignal::interruptGoto(args[0]);
  }
}

void
CwshShellCommandMgr::
popdCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "popd        ; pop directory stack" << std::endl;
    std::cout << "popd +<num> ; pop directory stack by num" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args > 0 && args[0][0] == '+') {
    if (num_args > 1)
      CWSH_THROW("Too many arguments.");

    string arg = args[0].substr(1);

    if (! CStrUtil::isInteger(arg))
      CWSH_THROW("Invalid argument.");

    int num = CStrUtil::toInteger(arg);

    if (num > cwsh->sizeDirStack())
      CWSH_THROW("Directory stack not that deep.");

    string dirname = cwsh->popDirStack(num);

    cwsh->changeDir(dirname);
  }
  else {
    if (num_args > 0)
      CWSH_THROW("Too many arguments.");

    if (cwsh->sizeDirStack() == 0)
      CWSH_THROW("Directory stack empty.");

    string dirname = cwsh->popDirStack();

    cwsh->changeDir(dirname);
  }

  cwsh->printDirStack();
}

void
CwshShellCommandMgr::
printenvCmd(Cwsh *, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "printenv        ; print all environment variables" << std::endl;
    std::cout << "printenv <name> ; print named environment variable" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args > 1)
    CWSH_THROW("Too many arguments.");

  if (num_args == 1) {
    if (CEnvInst.exists(args[0]))
      std::cout << CEnvInst.get(args[0]) << std::endl;
    else
      CWSH_THROW("Undefined variable.");
  }
  else {
    vector<string> names;
    vector<string> values;

    CEnvInst.getSortedNameValues(names, values);

    int num_names = names.size();

    for (int i = 0; i < num_names; i++)
      std::cout << names[i] << "=" << values[i] << std::endl;
  }
}

void
CwshShellCommandMgr::
pushdCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "pushd        ; change to directory on top of directory stack" << std::endl;
    std::cout << "pushd <name> ; push specified directory" << std::endl;
    std::cout << "pushd +<num> ; push to numbers directory on stack" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args > 1)
    CWSH_THROW("Too many arguments.");

  if (num_args > 0) {
    if (args[0][0] == '+') {
      string arg = args[0].substr(1);

      if (! CStrUtil::isInteger(arg))
        CWSH_THROW("Invalid argument.");

      int num = CStrUtil::toInteger(arg);

      if (cwsh->sizeDirStack() < num)
        CWSH_THROW("Directory stack not that deep.");

      string dirname = cwsh->popDirStack(num);

      cwsh->pushDirStack();

      cwsh->changeDir(dirname);
    }
    else {
      string dirname = args[0];

      dirname = CwshDir::lookup(cwsh, dirname);

      cwsh->pushDirStack();

      cwsh->changeDir(dirname);
    }
  }
  else {
    if (cwsh->sizeDirStack() < 1)
      CWSH_THROW("No other directory.");

    string dirname = cwsh->popDirStack();

    cwsh->pushDirStack();

    cwsh->changeDir(dirname);
  }

  cwsh->printDirStack();
}

void
CwshShellCommandMgr::
rehashCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "rehash ; rehash command lookup from path" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args != 0)
    CWSH_THROW("Too many arguments.");

  cwsh->setFilePathActive(true);

  cwsh->clearFilePath();
}

void
CwshShellCommandMgr::
repeatCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "repeat <n> <command> ; repeat command <n> times" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args < 2)
    CWSH_THROW("Too few arguments.");

  if (! CStrUtil::isInteger(args[0]))
    throw "repeat: Badly formed number.";

  int count = CStrUtil::toInteger(args[0]);

  vector<string> words;

  for (int i = 1; i < num_args; i++)
    words.push_back(args[i]);

  for (int i = 0; i < count; i++) {
    CAutoPtr<CwshCommandData> command;

    command = new CwshCommandData(cwsh, words);

    CwshCommand *command1 = command->getCommand();

    if (command1 != NULL) {
      command1->start();

      command1->wait();
    }
  }
}

void
CwshShellCommandMgr::
returnCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "return ; return from function" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args > 0)
    CWSH_THROW("Too many arguments.");

  CwshBlock *block = cwsh->findBlock(CWSH_BLOCK_TYPE_FUNCTION);

  if (block == NULL)
    CWSH_THROW("Not in function.");

  cwsh->setBlockReturn(true);
}

void
CwshShellCommandMgr::
setCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "set                     ; list variables" << std::endl;
    std::cout << "set <var> = <value>     ; set variable to value" << std::endl;
    std::cout << "set <var> = ( <value> ) ; set variable to array value" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args == 0) {
    cwsh->listVariables();

    return;
  }

  vector<string> args1;

  for (int i = 0; i < num_args; i++) {
    const string &arg = args[i];

    string::size_type pos = arg.find('=');

    if (pos != string::npos) {
      if (pos > 0)
        args1.push_back(arg.substr(0, pos));

      args1.push_back("=");

      if (pos + 1 < arg.size())
        args1.push_back(arg.substr(pos + 1));
    }
    else
      args1.push_back(arg);
  }

  int num_args1 = args1.size();

  int i1 = 0;

  while (i1 < num_args1) {
    string name = args1[i1];

    i1++;

    if (i1 < num_args1 && args1[i1] == "=") {
      i1++;

      if (i1 < num_args1) {
        if (args1[i1] == "(") {
          i1++;

          vector<string> values;

          while (i1 < num_args1 && args1[i1] != ")") {
            values.push_back(args1[i1]);

            i1++;
          }

          if (i1 < num_args1)
            i1++;

          cwsh->defineVariable(name, values);
        }
        else {
          cwsh->defineVariable(name, args1[i1]);

          i1++;
        }
      }
      else
        cwsh->defineVariable(name, "");
    }
    else
      cwsh->defineVariable(name, "");
  }
}

void
CwshShellCommandMgr::
setenvCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "setenv                ; print all environment variables" << std::endl;
    std::cout << "setenv <name>         ; print environment variable value" << std::endl;
    std::cout << "setenv <name> <value> ; set environment variable to value" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args > 2)
    CWSH_THROW("Too many arguments.");

  if (num_args == 0) {
    vector<string> names;
    vector<string> values;

    CEnvInst.getSortedNameValues(names, values);

    int num_names = names.size();

    for (int i = 0; i < num_names; i++)
      std::cout << names[i] << "=" << values[i] << std::endl;

    return;
  }

  const string &name = args[0];

  string value = "";

  if (num_args == 2)
    value = args[1];

  if (cwsh->isEnvironmentVariableUpper(name)) {
    string name1 = CStrUtil::toLower(name);

    CStrWords words = CStrUtil::toFields(value, ":");

    if (words.size() > 1) {
      vector<string> values;

      int num_words = words.size();

      for (int i = 0; i < num_words; i++)
        values.push_back(words[i].getWord());

      cwsh->defineVariable(name1, values);
    }
    else
      cwsh->defineVariable(name1, value);
  }
  else
    CEnvInst.set(name, value);
}

void
CwshShellCommandMgr::
shiftCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "shift        ; shift out next value from argv" << std::endl;
    std::cout << "shift <name> ; shift out next value from array variable" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args > 1)
    CWSH_THROW("Too many arguments.");

  string name = "argv";

  if (num_args == 1)
    name = args[0];

  CwshVariable *variable = cwsh->lookupVariable(name);

  if (variable == NULL)
    CWSH_THROW("Undefined variable.");

  if (variable->getNumValues() <= 0)
    CWSH_THROW("No more words.");

  variable->shift();
}

void
CwshShellCommandMgr::
sourceCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "source [-h] <file> ... ; source specified files" << std::endl;
    return;
  }

  int num_args = args.size();

  bool   history  = false;
  string filename = "";

  for (int i = 0; i < num_args; i++) {
    if (args[i][0] == '-') {
      if (args[i] == "-h")
        history = true;
      else
        CWSH_THROW("Invalid Option " + args[i] + ".");
    }
    else {
      if (filename == "")
        filename = args[i];
      else
        CWSH_THROW("Too many arguments.");
    }
  }

  if (filename == "")
    CWSH_THROW("Too few arguments.");

  if (history)
    cwsh->addHistoryFile(filename);
  else
    cwsh->executeInput(filename);
}

void
CwshShellCommandMgr::
stopCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "stop       ; stop current process" << std::endl;
    std::cout << "stop <job> ; stop specified job" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args > 0) {
    for (int i = 0; i < num_args; i++) {
      CwshProcess *process = cwsh->getActiveProcess(args[i]);

      if (process == NULL)
        CWSH_THROW("No such job.");

      process->stop();
    }
  }
  else {
    CwshProcess *process = cwsh->getCurrentActiveProcess();

    if (process == NULL)
      CWSH_THROW("No current job.");

    process->stop();
  }
}

void
CwshShellCommandMgr::
suspendCmd(Cwsh *, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "suspend ; suspend current process" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args > 0)
    CWSH_THROW("Too many arguments.");

  int pid = COSProcess::getProcessId();

  COSSignal::sendSignal(pid, SIGTSTP);
}

void
CwshShellCommandMgr::
switchCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "switch ( <expr> ) ; switch on specified expression value" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args != 3)
    CWSH_THROW("Syntax Error.");

  if (args[0] != "(" || args[2] != ")")
    CWSH_THROW("Syntax Error.");

  CwshLineArray lines;

  CwshShellCommand *command = cwsh->lookupShellCommand("switch");

  cwsh->getInputBlock(command, lines);

  bool case_processed = false;

  cwsh->startBlock(CWSH_BLOCK_TYPE_SWITCH, lines);

  bool processing = false;

  while (! cwsh->inputEof()) {
    string line = cwsh->getInputLine();

    vector<string> words;

    CwshString::addWords(line, words);

    if      (words.size() > 0 && words[0] == "case") {
      if (words.size() != 3 || words[2] != ":")
        CWSH_THROW("Syntax Error.");

      if (! case_processed) {
        CAutoPtr<CwshWildCard> wildcard;

        wildcard = new CwshWildCard(words[1]);

        if (wildcard->checkMatch(args[1]))
          processing = true;
        else
          processing = false;
      }
      else
        processing = false;
    }
    else if (words.size() > 0 && words[0] == "default") {
      if (! case_processed)
        processing = true;
      else
        processing = false;
    }
    else if (words.size() > 0 && words[0] == "breaksw")
      processing = false;
    else {
      if (processing) {
        cwsh->processInputLine(line);

        case_processed = true;
      }
    }

    if (cwsh->isBlockBreak   () ||
        cwsh->isBlockContinue() ||
        cwsh->isBlockReturn  ())
      break;

    if (cwsh->getBlockGotoDepth() > 0)
      break;
  }

  cwsh->endBlock();
}

void
CwshShellCommandMgr::
umaskCmd(Cwsh *, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "umask         ; print current umask" << std::endl;
    std::cout << "umask <value> ; set umask to value" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args > 1)
    CWSH_THROW("Too many arguments.");

  if (num_args == 0) {
    uint mask = COSUser::getUMask();

    int int1 = mask/64;
    int int2 = (mask - int1*64)/8;
    int int3 = mask - int1*64 - int2*8;

    std::cout << (char)(int1 + '0') <<
                 (char)(int2 + '0') <<
                 (char)(int3 + '0') << std::endl;
  }
  else {
    int len = args[0].size();

    if (len > 3)
      CWSH_THROW("Improper mask.");

    int int1 = 0;
    int int2 = 0;
    int int3 = 0;

    if      (len == 3) {
      int1 = args[0][2] - '0';
      int2 = args[0][1] - '0';
      int3 = args[0][0] - '0';
    }
    else if (len == 2) {
      int1 = args[0][1] - '0';
      int2 = args[0][0] - '0';
    }
    else if (len == 1)
      int1 = args[0][0] - '0';

    if (int1 < 0 || int1 > 7 || int2 < 0 || int2 > 7 || int3 < 0 || int3 > 7)
      CWSH_THROW("Improper mask.");

    uint mask = int3*64 + int2*8 + int1;

    COSUser::setUMask(mask);
  }
}

void
CwshShellCommandMgr::
timeCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "time <command> ; time command" << std::endl;
    return;
  }

  struct tms tms_data1;

  clock_t c1 = times(&tms_data1);

  string command = CStrUtil::toString(args, " ");

  cwsh->processInputLine(command);

  struct tms tms_data2;

  clock_t c2 = times(&tms_data2);

  long ticks = sysconf(_SC_CLK_TCK);

  printf("Elapsed = %lf secs, User = %lf secs, System = %lf secs\n",
         ((double) (c2 - c1))/ticks,
         ((double) (tms_data2.tms_utime - tms_data1.tms_utime))/ticks,
         ((double) (tms_data2.tms_stime - tms_data1.tms_stime))/ticks);
}

void
CwshShellCommandMgr::
unhashCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "unhash ; disable path command hashing" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args != 0)
    CWSH_THROW("Too many arguments.");

  cwsh->setFilePathActive(false);

  cwsh->clearFilePath();
}

void
CwshShellCommandMgr::
unaliasCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "unalias <name> ; undefine specified alias" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args < 1)
    CWSH_THROW("Too few arguments.");

  for (int i = 0; i < num_args; i++)
    cwsh->undefineAlias(args[i]);
}

void
CwshShellCommandMgr::
unlimitCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "unlimit                ; unlimit all resources" << std::endl;
    std::cout << "unlimit <resource> ... ; unlimit specified resource" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args == 0) {
    cwsh->unlimitAllResources();

    return;
  }

  for (int i = 0; i < num_args; i++)
    cwsh->unlimitResource(args[i]);
}

void
CwshShellCommandMgr::
unsetCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "unset <variable> ... ; unset specified variables" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args < 1)
    CWSH_THROW("Too few arguments.");

  for (int i = 0; i < num_args; i++)
    cwsh->undefineVariable(args[i]);
}

void
CwshShellCommandMgr::
unsetenvCmd(Cwsh *, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "unsetenv <variable> ... ; unset specified variables" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args < 1)
    CWSH_THROW("Too few arguments.");

  for (int i = 0; i < num_args; i++)
    if (CEnvInst.exists(args[i]))
      CEnvInst.unset(args[i]);
}

void
CwshShellCommandMgr::
waitCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "wait ; wait on active processes" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args > 0)
    CWSH_THROW("Too many arguments.");

  cwsh->waitActiveProcesses();
}

void
CwshShellCommandMgr::
whichCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "which [-a] <name> ; find alias, function or path of specified name" << std::endl;
    return;
  }

  int num_args = args.size();

  bool show_all = false;

  vector<string> files;

  for (int i = 0; i < num_args; i++) {
    if (args[i][0] == '-') {
      if (args[i] == "-a")
        show_all = true;
      else
        CWSH_THROW("Invalid Option " + args[i]);
    }
    else
      files.push_back(args[i]);
  }

  if (files.size() == 0)
    CWSH_THROW("Too few arguments.");

  int num_files = files.size();

  for (int i = 0; i < num_files; i++) {
    bool found = false;

    //------

    CwshAlias *alias = cwsh->lookupAlias(files[i]);

    if (alias != NULL) {
      found = true;

      std::cout << "alias: " << alias->getValue() << std::endl;

      if (! show_all)
        continue;
    }

    //------

    CwshFunction *function = cwsh->lookupFunction(files[i]);

    if (function != NULL) {
      found = true;

      std::cout << "function: " << files[i] << std::endl;

      if (! show_all)
        continue;
    }

    //------

    CwshShellCommand *command = cwsh->lookupShellCommand(files[i]);

    if (command != NULL) {
      found = true;

      std::cout << "builtin: " << files[i] << std::endl;

      if (! show_all)
        continue;
    }

    //------

    try {
      string path = CwshUnixCommand::search(cwsh, files[i]);

      found = true;

      std::cout << path << std::endl;

      if (! show_all)
        continue;
    }
    catch (...) {
    }

    //------

    if (! found)
      CWSH_THROW("no " + files[i] + " in (" + CEnvInst.get("PATH") + ")");
  }
}

void
CwshShellCommandMgr::
whileCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "while <expr> ; loop while expression is true" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args == 0)
    CWSH_THROW("Too few arguments.");

  string str = CStrUtil::toString(args, " ");

  string str1 = cwsh->processInputExprLine(str);

  uint i = 0;

  CwshExprParse parse(cwsh);

  string expr_str = parse.parse(str1, &i);

  CStrUtil::skipSpace(str1, &i);

  uint len1 = str1.size();

  if (i < len1)
    CWSH_THROW("Expression Syntax.");

  CwshShellCommand *command = cwsh->lookupShellCommand("while");

  CwshLineArray lines;

  cwsh->getInputBlock(command, lines);

  CwshExprEvaluate expr(cwsh, expr_str);

  int processing = expr.process();

  while (processing) {
    cwsh->startBlock(CWSH_BLOCK_TYPE_WHILE, lines);

    while (! cwsh->inputEof()) {
      string line = cwsh->getInputLine();

      cwsh->processInputLine(line);

      if (cwsh->isBlockBreak   () ||
          cwsh->isBlockContinue() ||
          cwsh->isBlockReturn  ())
        break;

      if (cwsh->getBlockGotoDepth() > 0) {
        cwsh->setBlockBreak(true);
        break;
      }

      if (cwsh->getInterrupt())
        break;
    }

    cwsh->endBlock();

    if (cwsh->isBlockBreak () ||
        cwsh->isBlockReturn())
      break;

    if (cwsh->getInterrupt())
      break;

    str1 = cwsh->processInputExprLine(str);

    i = 0;

    CwshExprParse parse(cwsh);

    expr_str = parse.parse(str1, &i);

    CStrUtil::skipSpace(str1, &i);

    uint len1 = str1.size();

    if (i < len1)
      CWSH_THROW("Expression Syntax.");

    CwshExprEvaluate expr(cwsh, expr_str);

    processing = expr.process();
  }

  cwsh->setBlockContinue(false);
  cwsh->setBlockBreak   (false);
}

void
CwshShellCommandMgr::
atCmd(Cwsh *cwsh, const CwshArgArray &args)
{
  if (isHelpArg(args)) {
    std::cout << "@                ; list all variables" << std::endl;
    std::cout << "@ <name> <value> ; set variable to numeric value" << std::endl;
    return;
  }

  int num_args = args.size();

  if (num_args == 0) {
    cwsh->listVariables();

    return;
  }

  string str = CStrUtil::toString(args, " ");

  string            name;
  int               index;
  string            expr_str;
  CwshSetAssignType assign_type;

  CwshSet set(cwsh);

  set.parseAssign(str, name, &index, &assign_type, expr_str);

  set.processAssign(name, index, assign_type, expr_str);
}

void
CwshShellCommandMgr::
badCmd(Cwsh *, const CwshArgArray &args)
{
  if (isHelpArg(args)) return;

  int num_args = args.size();

  for (int i = 0; i < num_args; i++)
    std::cerr << " " << args[i];

  std::cerr << std::endl;

  CWSH_THROW("unimplemented Command.");
}

bool
CwshShellCommandMgr::
isHelpArg(const CwshArgArray &args)
{
  uint num_args = args.size();

  for (uint i = 0; i < num_args; i++)
    if (args[i] == "--help")
      return true;

  return false;
}
