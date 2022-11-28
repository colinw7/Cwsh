#include <CwshI.h>
#include <COSProcess.h>
#include <COSSignal.h>
#include <CFileMatch.h>
#include <cerrno>

#include <sys/resource.h>
#include <sys/times.h>

namespace {

auto nSpace = [](int n) {
  std::string str;

  for (int i = 0; i < n; ++i)
    str += " ";

  return str;
};

auto helpStr = [](const std::string &cmd, const std::string &args, int len,
                  const std::string &desc) -> void {
  auto *mgr = CwshMgrInst;

  std::cout << mgr->helpNameColorStr() + cmd  + mgr->resetColorStr() + " " +
               mgr->helpArgsColorStr() + args + mgr->resetColorStr() +
               nSpace(len - int(args.size())) + " ; " +
               mgr->helpDescColorStr() + desc + mgr->resetColorStr() + "\n";
};

}

//---

#define CWSH_SHELL_CMD_DATA1(n,p) \
  { n , nullptr, &Cwsh::ShellCommandMgr::p, CWSH_SHELL_COMMAND_FLAGS_NONE }
#define CWSH_SHELL_CMD_DATA2(ns,ne,p) \
  { ns, ne     , &Cwsh::ShellCommandMgr::p, CWSH_SHELL_COMMAND_FLAGS_NONE }
#define CWSH_SHELL_CMD_DATA1_NW(n,p) \
  { n , nullptr, &Cwsh::ShellCommandMgr::p, CWSH_SHELL_COMMAND_FLAGS_NO_WILDCARDS }
#define CWSH_SHELL_CMD_DATA2_NE(ns,ne,p) \
  { ns, ne     , &Cwsh::ShellCommandMgr::p, CWSH_SHELL_COMMAND_FLAGS_NO_EXPAND }

namespace Cwsh {

ShellCommandData
ShellCommandMgr::
commandsData_[] = {
 CWSH_SHELL_CMD_DATA2("foreach", "end"    , foreachCmd),
 CWSH_SHELL_CMD_DATA2("func"   , "endfunc", funcCmd   ),
 CWSH_SHELL_CMD_DATA2("if"     , "endif"  , ifCmd     ),
 CWSH_SHELL_CMD_DATA2("switch" , "endsw"  , switchCmd ),

 CWSH_SHELL_CMD_DATA2_NE("while", "end", whileCmd  ),

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

ShellCommandMgr::
ShellCommandMgr(App *cwsh) :
 cwsh_(cwsh)
{
  uint num_commands = sizeof(commandsData_)/sizeof(commandsData_[0]);

  commands_.resize(num_commands);

  for (uint i = 0; i < num_commands; ++i)
    commands_[i] = new ShellCommand(cwsh_, &commandsData_[i]);
}

ShellCommandMgr::
~ShellCommandMgr()
{
  for (auto &command : commands_)
    delete command;
}

ShellCommand *
ShellCommandMgr::
lookup(const std::string &name) const
{
  if (name.size() > 0 && name[0] == '\\')
    return lookup(name.substr(1));

  uint num_commands = uint(commands_.size());

  for (uint i = 0; i < num_commands; i++)
    if (commands_[i]->getName() == name)
      return commands_[i];

  return nullptr;
}

void
ShellCommandMgr::
runProc(const ArgArray &args, CCommand::CallbackData data)
{
  auto *shell_command = reinterpret_cast<ShellCommand *>(data);

  if (shell_command->getCwsh()->getDebug()) {
    std::cerr << shell_command->getName();

    int num_args = int(args.size());

    for (int i = 0; i < num_args; i++)
      std::cerr << " " << args[i];

    std::cerr << "\n";
  }

  (shell_command->getProc())(shell_command->getCwsh(), args);
}

void
ShellCommandMgr::
colonCmd(App *, const ArgArray &args)
{
  if (isHelpArg(args))
    return;
}

void
ShellCommandMgr::
aliasCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("alias", "[-a]              ", 18, "list all aliases");
    helpStr("alias", "[-a] <name>       ", 18, "list named alias");
    helpStr("alias", "[-a] <name> <args>", 18, "define alias to args");
    return;
  }

  bool all = false;

  std::vector<std::string> args1;

  for (const auto &arg : args) {
    if (args1.empty() && arg[0] == '-') {
      if (arg == "-a")
        all = true;
      else
        std::cerr << "Invalid option: " << arg << "\n";
    }
    else
      args1.push_back(arg);
  }

  int num_args = int(args1.size());

  if      (num_args == 0) {
    cwsh->displayAliases(all);
  }
  else if (num_args == 1) {
    auto *alias = cwsh->lookupAlias(args1[0]);

    if (alias)
      alias->displayValue(all);
  }
  else {
    std::string cmd = CStrUtil::toString(args1, 1);

    cwsh->defineAlias(args1[0], cmd);
  }
}

void
ShellCommandMgr::
autoExecCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("autoexec", ""               , 16, "list all auto exec rules");
    helpStr("autoexec", "<suffix>"       , 16, "list named auto exec");
    helpStr("autoexec", "<suffix> <args>", 16, "define auto exec for suffix to args");
    return;
  }

  int num_args = int(args.size());

  if      (num_args == 0)
    cwsh->displayAutoExec();
  else if (num_args == 1) {
    auto *autoExec = cwsh->lookupAutoExec(args[0]);

    if (autoExec)
      std::cout << autoExec->getValue() << "\n";
  }
  else {
    std::string cmd = CStrUtil::toString(args, 1);

    cwsh->defineAutoExec(args[0], cmd);
  }
}

void
ShellCommandMgr::
bgCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("bg", ""         , 9, "move current process to background");
    helpStr("bg", "<job> ...", 9, "move specified jobs to background");
    return;
  }

  int num_args = int(args.size());

  if (num_args > 0) {
    for (int i = 0; i < num_args; i++) {
      auto *process = cwsh->getActiveProcess(args[i]);

      if (! process)
        CWSH_THROW("No such job.");

      std::cout << "[" << process->getNum() << "]    ";

      process->print();

      std::cout << " &\n";

      process->resume();
    }
  }
  else {
    auto *process = cwsh->getCurrentActiveProcess();

    if (! process)
      CWSH_THROW("No current job.");

    std::cout << "[" << process->getNum() << "]    ";

    process->print();

    std::cout << " &\n";

    process->resume();
  }
}

void
ShellCommandMgr::
breakCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("break", "", 0, "break out of while/foreach");
    return;
  }

  int num_args = int(args.size());

  if (num_args > 0)
    CWSH_THROW("Too many arguments.");

  auto *block = cwsh->findBlock(BlockType::WHILE);

  if (! block)
    block = cwsh->findBlock(BlockType::FOREACH);

  if (! block)
    CWSH_THROW("Not in while/foreach.");

  cwsh->setBlockBreak(true);
}

void
ShellCommandMgr::
breakswCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("breaksw", "", 0, "break out of switch");
    return;
  }

  int num_args = int(args.size());

  if (num_args > 0)
    CWSH_THROW("Too many arguments.");

  auto *block = cwsh->findBlock(BlockType::SWITCH);

  if (! block)
    CWSH_THROW("Not in switch.");

  cwsh->setBlockBreakSwitch(true);
}

void
ShellCommandMgr::
caseCmd(App *, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("case", "<expr>", 6, "switch case statement");
    return;
  }

  int num_args = int(args.size());

  if (num_args != 2)
    CWSH_THROW("Syntax Error.");

  if (args[1] != ":")
    CWSH_THROW("Syntax Error.");

  CWSH_THROW("Not in switch.");
}

void
ShellCommandMgr::
cdCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("cd", ""         , 9, "change to home directory");
    helpStr("cd", "<dir>"    , 9, "change to specified directory");
    helpStr("cd", "<dir> ...", 9, "change to first valid directory");
    return;
  }

  auto num_args = args.size();

  if (num_args == 0) {
    std::string dirname = COSUser::getUserHome();

    cwsh->changeDir(dirname);
  }
  else {
    for (size_t i = 0; i < num_args; ++i) {
      auto dirname = CStrUtil::stripSpaces(args[i]);

      if (dirname == "")
        dirname = COSUser::getUserHome();

      if (i == num_args - 1)
        dirname = Dir::lookup(cwsh, dirname, true);
      else {
        dirname = Dir::lookup(cwsh, dirname, false);

        if (dirname == "") continue;
      }

      cwsh->changeDir(dirname);
    }
  }
}

void
ShellCommandMgr::
completeCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("complete", "[-all|-best|-command|-file|-var] <pattern> ...", 0, "complete pattern");
    return;
  }

  enum class CompleteShow {
    ALL,
    BEST
  };

  enum class CompleteType {
    COMMAND,
    FILE,
    VAR
  };

  std::vector<std::string> args1;

  CompleteShow show = CompleteShow::ALL;
  CompleteType type = CompleteType::COMMAND;

  int num_args = int(args.size());

  for (int i = 0; i < num_args; i++) {
    if (args[i][0] == '-') {
      std::string name = args[i].substr(1);

      if      (name == "all")
        show = CompleteShow::ALL;
      else if (name == "best")
        show = CompleteShow::BEST;
      else if (name == "command")
        type = CompleteType::COMMAND;
      else if (name == "file")
        type = CompleteType::FILE;
      else if (name == "var")
        type = CompleteType::VAR;
      else
        CWSH_THROW("Invalid argument.");
    }
    else
      args1.push_back(args[i]);
  }

  int num_args1 = int(args1.size());

  if (num_args1 < 1)
    CWSH_THROW("Too few arguments.");

  if (num_args1 > 1)
    CWSH_THROW("Too many arguments.");

  if (show == CompleteShow::ALL) {
    std::string pattern_str = args1[0] + "*";

    std::vector<std::string> names;

    if      (type == CompleteType::COMMAND) {
      Pattern pattern(cwsh, pattern_str);

      pattern.expandPath(names);
    }
    else if (type == CompleteType::FILE) {
      CFileMatch fileMatch;

      fileMatch.matchPattern(pattern_str, names);
    }
    else {
      Pattern pattern(cwsh, pattern_str);

      pattern.expandVar(names);
    }

    int num_names = int(names.size());

    for (int i = 0; i < num_names; i++)
      std::cout << names[i] << "\n";
  }
  else {
    std::string word, word1;

    Complete complete(cwsh, args1[0]);

    bool flag;

    if      (type == CompleteType::COMMAND)
      flag = complete.completeCommand (word, word1);
    else if (type == CompleteType::FILE)
      flag = complete.completeFile    (word);
    else
      flag = complete.completeVariable(word);

    if (! flag)
      cwsh->beep();

    if (word1 != "")
      std::cout << word << " : " << word1 << "\n";
    else
      std::cout << word << "\n";
  }
}

void
ShellCommandMgr::
continueCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("continue", "", 0, "continue to next iteration of while/foreach");
    return;
  }

  int num_args = int(args.size());

  if (num_args > 0)
    CWSH_THROW("Too many arguments.");

  auto *block = cwsh->findBlock(BlockType::WHILE);

  if (! block)
    block = cwsh->findBlock(BlockType::FOREACH);

  if (! block)
    CWSH_THROW("Not in while/foreach.");

  cwsh->setBlockContinue(true);
}

void
ShellCommandMgr::
defaultCmd(App *, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("default", "", 0, "switch default statement");
    return;
  }

  int num_args = int(args.size());

  if (num_args != 1)
    CWSH_THROW("Syntax Error.");

  if (args[0] != ":")
    CWSH_THROW("Syntax Error.");

  CWSH_THROW("Not in switch.");
}

void
ShellCommandMgr::
dirsCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("dirs", "[-l]", 0, "print directory stack");
    return;
  }

  int num_args = int(args.size());

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
ShellCommandMgr::
echoCmd(App *, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("echo", "[-n] args", 0, "print arguments");
    return;
  }

  int num_args = int(args.size());

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
    std::cout << "\n";

  std::cout.flush();
}

void
ShellCommandMgr::
elseCmd(App *, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("else", "", 0, "else clause of if");
    return;
  }

  CWSH_THROW("Not in if.");
}

void
ShellCommandMgr::
endCmd(App *, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("end", "", 0, "end of while/foreach");
    return;
  }

  int num_args = int(args.size());

  if (num_args > 0)
    CWSH_THROW("Too many arguments.");

  CWSH_THROW("Not in while/foreach.");
}

void
ShellCommandMgr::
endfuncCmd(App *, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("endfunc", "", 0, "end of func");
    return;
  }

  int num_args = int(args.size());

  if (num_args > 0)
    CWSH_THROW("Too many arguments.");

  CWSH_THROW("Not in func.");
}

void
ShellCommandMgr::
endifCmd(App *, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("endif", "", 0, "end of if");
    return;
  }

  int num_args = int(args.size());

  if (num_args > 0)
    CWSH_THROW("Too many arguments.");

  CWSH_THROW("Not in if.");
}

void
ShellCommandMgr::
endswCmd(App *, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("endsw", "", 0, "end of switch");
    return;
  }

  int num_args = int(args.size());

  if (num_args > 0)
    CWSH_THROW("Too many arguments.");

  CWSH_THROW("Not in switch.");
}

void
ShellCommandMgr::
evalCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("eval", "<arg> ...", 0, "evaluate arg as if entered as input");
    return;
  }

  int num_args = int(args.size());

  if (num_args == 0)
    CWSH_THROW("Too few arguments.");

  std::string line = CStrUtil::toString(args, " ");

  cwsh->processInputLine(line);
}

void
ShellCommandMgr::
execCmd(App *, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("exec", "<arg> ...", 0, "execute command to replace shell");
    return;
  }

  int num_args = int(args.size());

  if (num_args == 0)
    CWSH_THROW("Too few arguments.");

  std::vector<char *> cargs;

  cargs.resize(num_args + 1);

  int i = 0;

  for ( ; i < num_args; i++)
    cargs[i] = const_cast<char *>(args[i].c_str());
  cargs[i] = nullptr;

  execvp(cargs[0], &cargs[0]);

  _exit(255);
}

void
ShellCommandMgr::
exitCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("exit", ""          , 10, "exit shell");
    helpStr("exit", "<expr> ...", 10, "exit shell with specified return value");
    return;
  }

  int num_args = int(args.size());

  int status = 1;

  if (num_args > 0) {
    std::string expr_str = CStrUtil::toString(args, " ");

    ExprEvaluate expr(cwsh, expr_str);

    status = expr.process();
  }
  else {
    auto *variable = cwsh->lookupVariable("status");

    if (variable && variable->getNumValues() == 1) {
      if (CStrUtil::isInteger(variable->getValue(0)))
        status = int(CStrUtil::toInteger(variable->getValue(0)));
    }
  }

  cwsh->setExit(true, status);
}

void
ShellCommandMgr::
exprCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("expr", "<expr> ...", 0, "evaluate expression");
    return;
  }

  std::string expr_str = CStrUtil::toString(args, " ");

  ExprEvaluate expr(cwsh, expr_str);

  std::cout << expr.process() << "\n";
}

void
ShellCommandMgr::
fgCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("fg", ""         , 9, "move current process to foreground");
    helpStr("fg", "<job> ...", 9, "move specified jobs to foreground");
    return;
  }

  uint num_args = int(args.size());

  if (num_args > 0) {
    for (uint i = 0; i < num_args; ++i) {
      auto *process = cwsh->getActiveProcess(args[i]);

      if (! process)
        CWSH_THROW("No such job.");

      process->print();

      std::cout << "\n";

      process->resume();

      process->wait();
    }
  }
  else {
    auto *process = cwsh->getCurrentActiveProcess();

    if (! process)
      CWSH_THROW("No current job.");

    process->print();

    std::cout << "\n";

    process->resume();

    process->wait();
  }
}

void
ShellCommandMgr::
foreachCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("foreach", "<arg> (<expr>)", 0, "loop for each value of <expr> setting <arg> to value");
    return;
  }

  int num_args = int(args.size());

  if (num_args < 3)
    CWSH_THROW("Too few arguments.");

  std::string varname = args[0];

  if (args[1] != "(" || args[num_args - 1] != ")")
    CWSH_THROW("Words not parenthesized.");

  std::vector<std::string> values;

  for (int i = 2; i < num_args - 1; i++)
    values.push_back(args[i]);

  LineArray lines;

  auto *command = cwsh->lookupShellCommand("foreach");

  cwsh->getInputBlock(command, lines);

  int num_values = int(values.size());

  for (int i = 0; i < num_values; i++) {
    cwsh->defineVariable(varname, values[i]);

    cwsh->startBlock(BlockType::FOREACH, lines);

    while (! cwsh->inputEof()) {
      auto line = cwsh->getInputLine();

      cwsh->processInputLine(line.line);

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
ShellCommandMgr::
funcCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("func", "<name> <args>", 0, "define function <name>");
    return;
  }

  int num_args = int(args.size());

  if (num_args == 0) {
    cwsh->listAllFunctions();

    return;
  }

  if (num_args != 3)
    CWSH_THROW("Too few arguments.");

  if (args[1] != "(" || args[2] != ")")
    CWSH_THROW("Syntax Error.");

  auto *command = cwsh->lookupShellCommand("func");

  LineArray lines;

  cwsh->getInputBlock(command, lines);

  cwsh->defineFunction(args[0], lines);
}

void
ShellCommandMgr::
globCmd(App *, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("glob", "<arg> ...", 0, "expand supplied arguments");
    return;
  }

  int num_args = int(args.size());

  std::string str;

  for (int i = 0; i < num_args; i++)
    str += args[i];

  std::cout << str;
}

void
ShellCommandMgr::
gotoCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("goto", "<label>", 0, "goto specified label");
    return;
  }

  int num_args = int(args.size());

  if (num_args == 0)
    CWSH_THROW("Too few arguments.");

  if (num_args > 1)
    CWSH_THROW("Too many arguments.");

  cwsh->gotoBlockLabel(args[0]);
}

void
ShellCommandMgr::
hashstatCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("hashstat", "", 0, "display command hashing statistics");
    return;
  }

  int num_args = int(args.size());

  if (num_args != 0)
    CWSH_THROW("Too many arguments.");

  cwsh->printFilePathStats();
}

void
ShellCommandMgr::
helpCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("help", ""     , 0, "display commands");
    helpStr("help", "<cmd>", 0, "display help for specified command");
    return;
  }

  bool show_all = false;

  std::vector<std::string> cmds;

  uint num_args = int(args.size());

  for (uint i = 0; i < num_args; ++i) {
    if (args[i][0] == '-') {
      std::string name = args[i].substr(1);

      if      (name == "a")
        show_all = true;
      else
        CWSH_THROW("Invalid argument.");
    }
    else
      cmds.push_back(args[i]);
  }

  auto *mgr = cwsh->getShellCommandMgr();

  uint num_cmds = uint(cmds.size());

  if (num_cmds == 0) {
    std::set<std::string> cmds1;

    uint num_commands = uint(mgr->commands_.size());

    uint i = 0;

    for ( ; i < num_commands; ++i)
      cmds1.insert(mgr->commands_[i]->getName());

    i = 0;

    for (const auto &cmd : cmds1) {
      if (show_all) {
        auto *command = mgr->lookup(cmd);

        ArgArray hargs;

        hargs.push_back("--help");

        (command->getProc())(cwsh, hargs);
      }
      else {
        if (i > 0) std::cout << " ";

        std::cout << cmd;
      }

      ++i;
    }

    if (! show_all)
      std::cout << "\n";
  }
  else {
    for (uint i = 0; i < num_cmds; ++i) {
      auto *command = mgr->lookup(args[i]);

      if (command) {
        ArgArray hargs;

        hargs.push_back("--help");

        (command->getProc())(cwsh, hargs);
      }
      else
        CWSH_THROW("Unknown command " + args[0]);
    }
  }
}

void
ShellCommandMgr::
historyCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("history", "[-h|-r]", 7, "display history");
    helpStr("history", "<num>"  , 7, "display numbered history event");
    return;
  }

  int num_args = int(args.size());

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

      num = int(CStrUtil::toInteger(args[i]));
    }
  }

  cwsh->displayHistory(num, show_numbers, show_time, reverse);
}

void
ShellCommandMgr::
ifCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("if", "(<expr>) then", 0, "start of if statement");
    return;
  }

  int num_args = int(args.size());

  if (num_args == 0)
    CWSH_THROW("Too few arguments.");

  std::string str = CStrUtil::toString(args, " ");

  uint i = 0;

  ExprParse parse(cwsh);

  std::string expr_str = parse.parse(str, &i);

  CStrUtil::skipSpace(str, &i);

  uint j = i;

  CStrUtil::skipNonSpace(str, &j);

  std::string word = str.substr(i, j - i);

  if (word == "then") {
    i = j;

    CStrUtil::skipSpace(str, &i);

    uint len = uint(str.size());

    if (i < len)
      CWSH_THROW("Improper then.");

    auto *command = cwsh->lookupShellCommand("if");

    LineArray lines;

    cwsh->getInputBlock(command, lines);

    if (cwsh->getDebug()) {
      std::cerr << "if ( " << expr_str << " ) then\n";

      for (const auto &line : lines)
        std::cerr << line.line << "\n";

      std::cerr << "endif\n";
    }

    ExprEvaluate expr(cwsh, expr_str);

    int processing = expr.process();

    bool if_processed = processing;

    cwsh->startBlock(BlockType::IF, lines);

    while (! cwsh->inputEof()) {
      auto line = cwsh->getInputLine();

      std::vector<std::string> words;

      String::addWords(line.line, words);

      if (words.size() > 0 && words[0] == "else") {
        if (words.size() > 1 && words[1] == "if") {
          std::string str1 = CStrUtil::toString(words, 2, -1);

          uint i1 = 0;

          ExprParse parse1(cwsh);

          std::string exprStr1 = parse1.parse(str1, &i1);

          CStrUtil::skipSpace(str1, &i1);

          uint j1 = i1;

          CStrUtil::skipNonSpace(str1, &j1);

          std::string word1 = str1.substr(i1, j1 - i1);

          if (word1 == "then") {
            i1 = j1;

            CStrUtil::skipSpace(str1, &i1);

            uint len1 = uint(str1.size());

            if (i1 < len1)
              CWSH_THROW("Improper then.");

            ExprEvaluate expr1(cwsh, exprStr1);

            int processing1 = expr1.process();

            if (! if_processed)
              processing = processing1;

            if (processing)
              if_processed = true;
          }
          else {
            std::string line1 = str1.substr(i1);

            ExprEvaluate expr1(cwsh, exprStr1);

            int processing1 = expr1.process();

            if (! if_processed)
              processing = processing1;
            else
              processing = false;

            if (processing)
              if_processed = true;

            if (processing) {
              cwsh->processInputLine(line1);

              processing = false;
            }
          }
        }
        else {
          processing = ! if_processed;

          if (processing)
            if_processed = true;

          if (words.size() > 1) {
            std::string line1 = CStrUtil::toString(words, 1, -1);

            cwsh->processInputLine(line1);

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
    std::string line = str.substr(i);

    if (cwsh->getDebug())
      std::cerr << "if ( " << expr_str << " ) " << line << "\n";

    ExprEvaluate expr(cwsh, expr_str);

    int processing = expr.process();

    if (processing)
      cwsh->processInputLine(line);
  }
}

void
ShellCommandMgr::
jobsCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("jobs", "[-l]", 0, "display jobs");
    return;
  }

  int num_args = int(args.size());

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
ShellCommandMgr::
killCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("kill", "-l"                      , 24, "list signals");
    helpStr("kill", "[-<num>|-<signal>] <pid>", 24, "kill process");
    return;
  }

  int num_args = int(args.size());

  bool list_signals = false;

  int signal_num = -1;

  int i = 0;

  for ( ; i < num_args; i++) {
    if      (args[i] == "-l")
      list_signals = true;
    else if (args[i].size() > 0 && args[i][0] == '-') {
      std::string arg = args[i].substr(1);

      if (args[i].size() > 1 && isdigit(args[i][1])) {
        if (! CStrUtil::isInteger(arg))
          CWSH_THROW("Badly formed number.");

        signal_num = int(CStrUtil::toInteger(arg));

        auto *signal = Signal::lookup(signal_num);

        if (! signal)
          CWSH_THROW("Bad signal number.");
      }
      else {
        auto *signal = Signal::lookup(arg);

        if (! signal)
          CWSH_THROW("Unknown signal name.");

        signal_num = signal->getNum();
      }
    }
    else
      break;
  }

  if (list_signals) {
    int num_signals = Signal::getNumSignals();

    for (int j = 0; j < num_signals; j++) {
      auto *signal = Signal::getSignal(j);

      std::cout << signal->getName() << " ";
    }

    std::cout << "\n";

    return;
  }

  if (signal_num == -1)
    signal_num = SIGTERM;

  if (i >= num_args)
    CWSH_THROW("Too few arguments.");

  std::vector<int> pids;

  for ( ; i < num_args; i++) {
    int pid;

    if (args[i].size() > 0 && args[i][0] == '%')
      pid = cwsh->stringToProcessId(args[i]);
    else {
      if (! CStrUtil::isInteger(args[i]))
        CWSH_THROW("Arguments should be jobs or process id's.");

      pid = int(CStrUtil::toInteger(args[i]));
    }

    pids.push_back(pid);
  }

  uint num_pids = uint(pids.size());

  for (uint pi = 0; pi < num_pids; ++pi)
    cwsh->killProcess(pids[pi], signal_num);
}

void
ShellCommandMgr::
limitCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("limit", "[-h] <name> <value>", 0, "set named limit ti value");
    return;
  }

  int num_args = int(args.size());

  bool        hard  = false;
  std::string name  = "";
  std::string value = "";

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
ShellCommandMgr::
niceCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("nice", ""                     , 22, "get priority of current process");
    helpStr("nice", "[+<num>|-<num>"       , 22, "increase/decrease priority of current process");
    helpStr("nice", "[+<num>|-<num>] <pid>", 22, "run command as specified priority");
    return;
  }

  int num_args = int(args.size());

  int pid = COSProcess::getProcessId();

  errno = 0;

  int priority = getpriority(PRIO_PROCESS, pid);

  if (priority == -1 && errno != 0)
    CWSH_THROW("getpriority failed.");

  if (num_args == 0) {
    std::cout << "Current Priority " << priority << "\n";

    return;
  }

  int dpriority = 0;

  int i = 0;

  if (num_args > 0 && (args[i][0] == '+' || args[i][0] == '-')) {
    std::string istr = args[i].substr(1);

    if (! CStrUtil::isInteger(istr))
      CWSH_THROW("Invalid argument.");

    dpriority = int(CStrUtil::toInteger(istr));

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

    std::string command = CStrUtil::toString(args, i);

    cwsh->processInputLine(command);
  }
  else {
    int error = nice(dpriority);

    if (error < 0)
      CWSH_THROW("nice failed.");
  }
}

void
ShellCommandMgr::
nohupCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("nohup", ""         , 9, "set no hangup");
    helpStr("nohup", "<command>", 9, "run command with no hangup");
    return;
  }

  int num_args = int(args.size());

  if (num_args == 0) {
    Signal::nohup();
  }
  else {
    // TODO: start command with SIGHUP disabled

    CWSH_THROW("Not implemented.");

    std::string command = CStrUtil::toString(args, " ");

    cwsh->processInputLine(command);
  }
}

void
ShellCommandMgr::
notifyCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("notify", ""     , 5, "notify current processs");
    helpStr("notify", "<pid>", 5, "notify processs");
    return;
  }

  int num_args = int(args.size());

  if (num_args > 0) {
    for (int i = 0; i < num_args; i++) {
      auto *process = cwsh->getActiveProcess(args[i]);

      if (! process)
        CWSH_THROW("No such job.");

      process->setNotify(true);
    }
  }
  else {
    auto *process = cwsh->getCurrentActiveProcess();

    if (! process)
      CWSH_THROW("No current job.");

    process->setNotify(true);
  }
}

void
ShellCommandMgr::
onintrCmd(App *, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("onintr", ""       , 7, "reset interrupts");
    helpStr("onintr", "-"      , 7, "ignore interrupts");
    helpStr("onintr", "<label>", 7, "goto label on interrupt");
    return;
  }

  int num_args = int(args.size());

  if (num_args > 1)
    CWSH_THROW("Too many arguments.");

  if (num_args == 0) {
    Signal::interruptReset();
  }
  else {
    if (args[0] == "-")
      Signal::interruptIgnore();
    else
      Signal::interruptGoto(args[0]);
  }
}

void
ShellCommandMgr::
popdCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("popd", ""      , 6, "pop directory stack");
    helpStr("popd", "+<num>", 6, "pop directory stack by num");
    return;
  }

  int num_args = int(args.size());

  if (num_args > 0 && args[0][0] == '+') {
    if (num_args > 1)
      CWSH_THROW("Too many arguments.");

    std::string arg = args[0].substr(1);

    if (! CStrUtil::isInteger(arg))
      CWSH_THROW("Invalid argument.");

    int num = int(CStrUtil::toInteger(arg));

    if (num > cwsh->sizeDirStack())
      CWSH_THROW("Directory stack not that deep.");

    std::string dirname = cwsh->popDirStack(num);

    cwsh->changeDir(dirname);
  }
  else {
    if (num_args > 0)
      CWSH_THROW("Too many arguments.");

    if (cwsh->sizeDirStack() == 0)
      CWSH_THROW("Directory stack empty.");

    std::string dirname = cwsh->popDirStack();

    cwsh->changeDir(dirname);
  }

  cwsh->printDirStack();
}

void
ShellCommandMgr::
printenvCmd(App *, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("printenv", ""      , 6, "print all environment variables");
    helpStr("printenv", "<name>", 6, "print named environment variable");
    return;
  }

  int num_args = int(args.size());

  if (num_args > 1)
    CWSH_THROW("Too many arguments.");

  auto *mgr = CwshMgrInst;

  if (num_args == 1) {
    if (CEnvInst.exists(args[0]))
      std::cout << CEnvInst.get(args[0]) << "\n";
    else
      CWSH_THROW("Undefined variable.");
  }
  else {
    std::vector<std::string> names;
    std::vector<std::string> values;

    CEnvInst.getSortedNameValues(names, values);

    int num_names = int(names.size());

    for (int i = 0; i < num_names; i++) {
      std::cout << mgr->envNameColorStr () << names [i] << mgr->resetColorStr() << "=" <<
                   mgr->envValueColorStr() << values[i] << mgr->resetColorStr() << "\n";
    }
  }
}

void
ShellCommandMgr::
pushdCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("pushd", ""      , 6, "change to directory on top of directory stack");
    helpStr("pushd", "<name>", 6, "push specified directory");
    helpStr("pushd", "+<num>", 6, "push to numbers directory on stack");
    return;
  }

  int num_args = int(args.size());

  if (num_args > 1)
    CWSH_THROW("Too many arguments.");

  if (num_args > 0) {
    if (args[0][0] == '+') {
      std::string arg = args[0].substr(1);

      if (! CStrUtil::isInteger(arg))
        CWSH_THROW("Invalid argument.");

      int num = int(CStrUtil::toInteger(arg));

      if (cwsh->sizeDirStack() < num)
        CWSH_THROW("Directory stack not that deep.");

      std::string dirname = cwsh->popDirStack(num);

      cwsh->pushDirStack();

      cwsh->changeDir(dirname);
    }
    else {
      std::string dirname = args[0];

      dirname = Dir::lookup(cwsh, dirname);

      cwsh->pushDirStack();

      cwsh->changeDir(dirname);
    }
  }
  else {
    if (cwsh->sizeDirStack() < 1)
      CWSH_THROW("No other directory.");

    std::string dirname = cwsh->popDirStack();

    cwsh->pushDirStack();

    cwsh->changeDir(dirname);
  }

  cwsh->printDirStack();
}

void
ShellCommandMgr::
rehashCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("rehash", "", 0, "rehash command lookup from path");
    return;
  }

  int num_args = int(args.size());

  if (num_args != 0)
    CWSH_THROW("Too many arguments.");

  cwsh->setFilePathActive(true);

  cwsh->clearFilePath();
}

void
ShellCommandMgr::
repeatCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("repeat", "<n> <command>", 0, "repeat command <n> times");
    return;
  }

  int num_args = int(args.size());

  if (num_args < 2)
    CWSH_THROW("Too few arguments.");

  if (! CStrUtil::isInteger(args[0]))
    throw "repeat: Badly formed number.";

  int count = int(CStrUtil::toInteger(args[0]));

  std::vector<std::string> words;

  for (int i = 1; i < num_args; i++)
    words.push_back(args[i]);

  for (int i = 0; i < count; i++) {
    CAutoPtr<CommandData> command;

    command = new CommandData(cwsh, words);

    auto *command1 = command->getCommand();

    if (command1) {
      command1->start();

      command1->wait();
    }
  }
}

void
ShellCommandMgr::
returnCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("return", "", 0, "return from function");
    return;
  }

  int num_args = int(args.size());

  if (num_args > 0)
    CWSH_THROW("Too many arguments.");

  auto *block = cwsh->findBlock(BlockType::FUNCTION);

  if (! block)
    CWSH_THROW("Not in function.");

  cwsh->setBlockReturn(true);
}

void
ShellCommandMgr::
setCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("set", ""                   , 19, "list variables");
    helpStr("set", "<var> = <value>"    , 19, "set variable to value");
    helpStr("set", "<var> = ( <value> )", 19, "set variable to array value");
    return;
  }

  int num_args = int(args.size());

  if (num_args == 0) {
    cwsh->listVariables(/*all*/ true);

    return;
  }

  std::vector<std::string> args1;

  for (int i = 0; i < num_args; i++) {
    const std::string &arg = args[i];

    std::string::size_type pos = arg.find('=');

    if (pos != std::string::npos) {
      if (pos > 0)
        args1.push_back(arg.substr(0, pos));

      args1.push_back("=");

      if (pos + 1 < arg.size())
        args1.push_back(arg.substr(pos + 1));
    }
    else
      args1.push_back(arg);
  }

  int num_args1 = int(args1.size());

  int i1 = 0;

  while (i1 < num_args1) {
    std::string name = args1[i1];

    i1++;

    if (i1 < num_args1 && args1[i1] == "=") {
      i1++;

      if (i1 < num_args1) {
        if (args1[i1] == "(") {
          i1++;

          std::vector<std::string> values;

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
ShellCommandMgr::
setenvCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("setenv", ""              , 14, "print all environment variables"  );
    helpStr("setenv", "<name>"        , 14, "print environment variable value" );
    helpStr("setenv", "<name> <value>", 14, "set environment variable to value");
    return;
  }

  //---

  int num_args = int(args.size());

  if (num_args > 2)
    CWSH_THROW("Too many arguments.");

  auto *mgr = CwshMgrInst;

  if (num_args == 0) {
    std::vector<std::string> names;
    std::vector<std::string> values;

    CEnvInst.getSortedNameValues(names, values);

    int num_names = int(names.size());

    for (int i = 0; i < num_names; i++) {
      std::cout <<
        mgr->envNameColorStr () << names [i] << mgr->resetColorStr() << "=" <<
        mgr->envValueColorStr() << values[i] << mgr->resetColorStr() << "\n";
    }

    return;
  }

  const std::string &name = args[0];

  std::string value = "";

  if (num_args == 2)
    value = args[1];

  if (cwsh->isEnvironmentVariableUpper(name)) {
    std::string name1 = CStrUtil::toLower(name);

    CStrWords words = CStrUtil::toFields(value, ":");

    if (words.size() > 1) {
      std::vector<std::string> values;

      int num_words = int(words.size());

      for (int i = 0; i < num_words; i++)
        values.push_back(words[i].getWord());

      cwsh->defineVariable(name1, values);
    }
    else
      cwsh->defineVariable(name1, value);
  }
  else {
    CEnvInst.set(name, value);
  }
}

void
ShellCommandMgr::
shiftCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("shift", ""      , 6, "shift out next value from argv");
    helpStr("shift", "<name>", 6, "shift out next value from array variable");
    return;
  }

  int num_args = int(args.size());

  if (num_args > 1)
    CWSH_THROW("Too many arguments.");

  std::string name = "argv";

  if (num_args == 1)
    name = args[0];

  auto *variable = cwsh->lookupVariable(name);

  if (! variable)
    CWSH_THROW("Undefined variable.");

  if (variable->getNumValues() <= 0)
    CWSH_THROW("No more words.");

  variable->shift();
}

void
ShellCommandMgr::
sourceCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("source", "[-h] <file> ...", 0, "source specified files");
    return;
  }

  int num_args = int(args.size());

  bool        history  = false;
  std::string filename = "";

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
ShellCommandMgr::
stopCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("stop", ""     , 5, "stop current process");
    helpStr("stop", "<job>", 5, "stop specified job");
    return;
  }

  int num_args = int(args.size());

  if (num_args > 0) {
    for (int i = 0; i < num_args; i++) {
      auto *process = cwsh->getActiveProcess(args[i]);

      if (! process)
        CWSH_THROW("No such job.");

      process->stop();
    }
  }
  else {
    auto *process = cwsh->getCurrentActiveProcess();

    if (! process)
      CWSH_THROW("No current job.");

    process->stop();
  }
}

void
ShellCommandMgr::
suspendCmd(App *, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("suspend", "", 0, "suspend current process");
    return;
  }

  int num_args = int(args.size());

  if (num_args > 0)
    CWSH_THROW("Too many arguments.");

  int pid = COSProcess::getProcessId();

  COSSignal::sendSignal(pid, SIGTSTP);
}

void
ShellCommandMgr::
switchCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("switch", "( <expr> )", 0, "switch on specified expression value");
    return;
  }

  int num_args = int(args.size());

  if (num_args != 3)
    CWSH_THROW("Syntax Error.");

  if (args[0] != "(" || args[2] != ")")
    CWSH_THROW("Syntax Error.");

  LineArray lines;

  auto *command = cwsh->lookupShellCommand("switch");

  cwsh->getInputBlock(command, lines);

  bool case_processed = false;

  cwsh->startBlock(BlockType::SWITCH, lines);

  bool processing = false;

  while (! cwsh->inputEof()) {
    auto line = cwsh->getInputLine();

    std::vector<std::string> words;

    String::addWords(line.line, words);

    if      (words.size() > 0 && words[0] == "case") {
      if (words.size() != 3 || words[2] != ":")
        CWSH_THROW("Syntax Error.");

      if (! case_processed) {
        CAutoPtr<WildCard> wildcard;

        wildcard = new WildCard(words[1]);

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
ShellCommandMgr::
umaskCmd(App *, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("umask", ""       , 7, "print current umask");
    helpStr("umask", "<value>", 7, "set umask to value");
    return;
  }

  int num_args = int(args.size());

  if (num_args > 1)
    CWSH_THROW("Too many arguments.");

  if (num_args == 0) {
    uint mask = COSUser::getUMask();

    int int1 = mask/64;
    int int2 = (mask - int1*64)/8;
    int int3 = mask - int1*64 - int2*8;

    std::cout << char(int1 + '0') << char(int2 + '0') << char(int3 + '0') << "\n";
  }
  else {
    int len = int(args[0].size());

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
ShellCommandMgr::
timeCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("time", "<command>", 0, "time command");
    return;
  }

  struct tms tms_data1;

  clock_t c1 = times(&tms_data1);

  std::string command = CStrUtil::toString(args, " ");

  cwsh->processInputLine(command);

  struct tms tms_data2;

  clock_t c2 = times(&tms_data2);

  long ticks = sysconf(_SC_CLK_TCK);

  printf("Elapsed = %lf secs, User = %lf secs, System = %lf secs\n",
         double((c2 - c1)/ticks),
         double((tms_data2.tms_utime - tms_data1.tms_utime)/ticks),
         double((tms_data2.tms_stime - tms_data1.tms_stime)/ticks));
}

void
ShellCommandMgr::
unhashCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("unhash", "", 0, "disable path command hashing");
    return;
  }

  int num_args = int(args.size());

  if (num_args != 0)
    CWSH_THROW("Too many arguments.");

  cwsh->setFilePathActive(false);

  cwsh->clearFilePath();
}

void
ShellCommandMgr::
unaliasCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("unalias", "<name>", 0, "undefine specified alias");
    return;
  }

  int num_args = int(args.size());

  if (num_args < 1)
    CWSH_THROW("Too few arguments.");

  for (int i = 0; i < num_args; i++)
    cwsh->undefineAlias(args[i]);
}

void
ShellCommandMgr::
unlimitCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("unlimit", ""              , 13, "unlimit all resources");
    helpStr("unlimit", "<resource> ...", 13, "unlimit specified resource");
    return;
  }

  int num_args = int(args.size());

  if (num_args == 0) {
    cwsh->unlimitAllResources();

    return;
  }

  for (int i = 0; i < num_args; i++)
    cwsh->unlimitResource(args[i]);
}

void
ShellCommandMgr::
unsetCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("unset", "<variable> ...", 0, "unset specified variables");
    return;
  }

  int num_args = int(args.size());

  if (num_args < 1)
    CWSH_THROW("Too few arguments.");

  for (int i = 0; i < num_args; i++)
    cwsh->undefineVariable(args[i]);
}

void
ShellCommandMgr::
unsetenvCmd(App *, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("unsetenv", "<variable> ...", 0, "unset specified variables");
    return;
  }

  int num_args = int(args.size());

  if (num_args < 1)
    CWSH_THROW("Too few arguments.");

  for (int i = 0; i < num_args; i++) {
    if (CEnvInst.exists(args[i]))
      CEnvInst.unset(args[i]);
  }
}

void
ShellCommandMgr::
waitCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("wait", "", 0, "wait on active processes");
    return;
  }

  int num_args = int(args.size());

  if (num_args > 0)
    CWSH_THROW("Too many arguments.");

  cwsh->waitActiveProcesses();
}

void
ShellCommandMgr::
whichCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("which", "[-a] <name>", 0, "find alias, function or path of specified name");
    return;
  }

  int num_args = int(args.size());

  bool show_all = false;

  std::vector<std::string> files;

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

  int num_files = int(files.size());

  for (int i = 0; i < num_files; i++) {
    bool found = false;

    //------

    auto *alias = cwsh->lookupAlias(files[i]);

    if (alias) {
      found = true;

      std::cout << "alias: ";

      alias->displayValue(/*all*/ true);

      if (! show_all)
        continue;
    }

    //------

    auto *function = cwsh->lookupFunction(files[i]);

    if (function) {
      found = true;

      std::cout << "function: " << files[i] << "\n";

      if (! show_all)
        continue;
    }

    //------

    auto *command = cwsh->lookupShellCommand(files[i]);

    if (command) {
      found = true;

      std::cout << "builtin: " << files[i] << "\n";

      if (! show_all)
        continue;
    }

    //------

    try {
      std::string path = UnixCommand::search(cwsh, files[i]);

      found = true;

      std::cout << path << "\n";

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
ShellCommandMgr::
whileCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("while", "<expr>", 0, "loop while expression is true");
    return;
  }

  int num_args = int(args.size());

  if (num_args == 0)
    CWSH_THROW("Too few arguments.");

  std::string str = CStrUtil::toString(args, " ");

  std::string str1 = cwsh->processInputExprLine(str);

  uint i = 0;

  ExprParse parse(cwsh);

  std::string expr_str = parse.parse(str1, &i);

  CStrUtil::skipSpace(str1, &i);

  uint len1 = uint(str1.size());

  if (i < len1)
    CWSH_THROW("Expression Syntax.");

  auto *command = cwsh->lookupShellCommand("while");

  LineArray lines;

  cwsh->getInputBlock(command, lines);

  ExprEvaluate expr(cwsh, expr_str);

  int processing = expr.process();

  while (processing) {
    cwsh->startBlock(BlockType::WHILE, lines);

    while (! cwsh->inputEof()) {
      auto line = cwsh->getInputLine();

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

    ExprParse parse1(cwsh);

    expr_str = parse1.parse(str1, &i);

    CStrUtil::skipSpace(str1, &i);

    uint len2 = uint(str1.size());

    if (i < len2)
      CWSH_THROW("Expression Syntax.");

    ExprEvaluate expr1(cwsh, expr_str);

    processing = expr1.process();
  }

  cwsh->setBlockContinue(false);
  cwsh->setBlockBreak   (false);
}

void
ShellCommandMgr::
atCmd(App *cwsh, const ArgArray &args)
{
  if (isHelpArg(args)) {
    helpStr("@", ""              , 14, "list all variables");
    helpStr("@", "<name> <value>", 14, "set variable to numeric value");
    return;
  }

  int num_args = int(args.size());

  if (num_args == 0) {
    cwsh->listVariables(/*all*/ true);

    return;
  }

  std::string str = CStrUtil::toString(args, " ");

  std::string   name;
  int           index;
  std::string   expr_str;
  SetAssignType assign_type;

  Set set(cwsh);

  set.parseAssign(str, name, &index, &assign_type, expr_str);

  set.processAssign(name, index, assign_type, expr_str);
}

void
ShellCommandMgr::
badCmd(App *, const ArgArray &args)
{
  if (isHelpArg(args)) return;

  int num_args = int(args.size());

  for (int i = 0; i < num_args; i++)
    std::cerr << " " << args[i];

  std::cerr << "\n";

  CWSH_THROW("unimplemented Command.");
}

bool
ShellCommandMgr::
isHelpArg(const ArgArray &args)
{
  uint num_args = int(args.size());

  for (uint i = 0; i < num_args; i++)
    if (args[i] == "--help")
      return true;

  return false;
}

}
