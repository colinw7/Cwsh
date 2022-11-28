#ifndef CWSH_H
#define CWSH_H

#define CwshMgrInst Cwsh::Mgr::getInstance()

#include <CConfig.h>
#include <CAutoPtr.h>

class CFile;
class CMessage;

namespace Cwsh {

enum class PromptType {
  NORMAL,
  EXTRA
};

}

//---

namespace Cwsh {

class App;

class Mgr {
 private:
  using AppList = std::list<App *>;

 public:
  static Mgr *getInstance();

 ~Mgr();

  const std::string &locationColorStr() const { return locationColor_; }

  const std::string &aliasNameColorStr () const { return aliasNameColor_ ; }
  const std::string &aliasValueColorStr() const { return aliasValueColor_; }

  const std::string &envNameColorStr () const { return envNameColor_ ; }
  const std::string &envValueColorStr() const { return envValueColor_; }

  const std::string &funcNameColorStr () const { return funcNameColor_ ; }
  const std::string &funcValueColorStr() const { return funcValueColor_; }

  const std::string &helpNameColorStr() const { return helpNameColor_; }
  const std::string &helpArgsColorStr() const { return helpArgsColor_; }
  const std::string &helpDescColorStr() const { return helpDescColor_; }

  const std::string &varNameColorStr () const { return varNameColor_ ; }
  const std::string &varValueColorStr() const { return varValueColor_; }

  const std::string &resetColorStr() const { return resetColor_; }

  void add   (App *cwsh);
  void remove(App *cwsh);

  void term(int status);
  void term(App *cwsh, int status);

  void setInterrupt(bool flag);
  void readInterrupt();

  void gotoBlockLabel(const std::string &label);

  void stopActiveProcesses();

 private:
  Mgr();

 private:
  CConfig     config_;
  AppList     cwshList_;
  std::string locationColor_;
  std::string aliasNameColor_;
  std::string aliasValueColor_;
  std::string envNameColor_;
  std::string envValueColor_;
  std::string funcNameColor_;
  std::string funcValueColor_;
  std::string helpNameColor_;
  std::string helpArgsColor_;
  std::string helpDescColor_;
  std::string varNameColor_;
  std::string varValueColor_;
  std::string resetColor_;
};

//---

class App {
 public:
  using MessageP = std::shared_ptr<CMessage>;

 public:
  App();
 ~App();

  void init();
  void init(int argc, char **argv);
  void term();

  bool processArgs(int argc, char **argv);
  void initEnv();

  void enableServer();

  static MessageP createServerMessage();

  void mainLoop();
  void processLine(const std::string &str);

  //---

  const std::string &getName         () const { return name_; }
  bool               getCompatible   () const { return compatible_; }
  bool               getSilentMode   () const { return silentMode_; }
  std::string        getArgv0        () const { return argv0_; }

  ShellCommandMgr *getShellCommandMgr() const { return shellCmdMgr_.get(); }

  bool changeDir(const std::string &dirname);

  void setFastStartup(bool flag=true) { fastStartup_ = flag; }

  // get/set filename
  void setFilename(const std::string &name) { filename_ = name; }
  const std::string &getFilename() const { return filename_; }

  // get/set line number
  void setLineNum(int num) { lineNum_ = num; }
  int getLineNum() const { return lineNum_; }

  // get/set prompt type
  PromptType getPromptType() const { return promptType_; }
  void setPromptType(PromptType type) { promptType_ = type; }

  // get/set prompt command
  const std::string &getPromptCommand() const { return promptCommand_; }
  void setPromptCommand(const std::string &command) { promptCommand_ = command; }

  // get/set interrupt
  bool getInterrupt() const { return interrupt_; }
  void setInterrupt(bool flag) { interrupt_ = flag; }

  // get/set debug
  bool getDebug() const { return debug_; }
  void setDebug(bool flag);

  // get/set exit
  bool getExit() const { return exit_; }
  void setExit(bool flag, int status=0) { exit_ = flag; exitStatus_ = status; }

  //---

  // Function

  Function *defineFunction(const std::string &name, const LineArray &lines);

  void undefineFunction(const std::string &name);

  Function *lookupFunction(const std::string &name);

  void listAllFunctions();

  //---

  // Variable

  Variable *defineVariable(const std::string &name);
  Variable *defineVariable(const std::string &name, const std::string &value);
  Variable *defineVariable(const std::string &name, int value);
  Variable *defineVariable(const std::string &name, const VariableValueArray &values);
  Variable *defineVariable(const std::string &name, const char **values, int numValues);

  void undefineVariable(const std::string &name);

  Variable *lookupVariable(const std::string &name) const;

  const VariableList &variables() const;

  void listVariables(bool all) const;

  void saveVariables();
  void restoreVariables();

  bool isEnvironmentVariableLower(const std::string &name);
  bool isEnvironmentVariableUpper(const std::string &name);

  void updateEnvironmentVariable(Variable *variable);

  //---

  // Process

  Process *addProcess(CommandData *command);

  void removeProcess(Process *process);

  void killProcess(int pid, int signal);

  int getNumActiveProcesses();

  void displayActiveProcesses(bool listPids);
  void displayExitedProcesses();

  void waitActiveProcesses();

  int stringToProcessId(const std::string &str);

  Process *getActiveProcess(const std::string &str);
  Process *getCurrentActiveProcess();

  Process *lookupProcess(pid_t pid);

  //---

  // State
  void saveState();
  void restoreState();

  //---

  // Block
  Block *startBlock(BlockType type, const LineArray &lines);
  void   endBlock();

  bool inBlock      () const;
  bool blockEof     () const;
  Line blockReadLine() const;

  Block *findBlock(BlockType type);

  void gotoBlockLabel(const std::string &label);

  bool isBlockBreak      () const;
  bool isBlockBreakSwitch() const;
  bool isBlockContinue   () const;
  bool isBlockReturn     () const;

  int getBlockGotoDepth() const;

  void setBlockBreak      (bool flag);
  void setBlockBreakSwitch(bool flag);
  void setBlockContinue   (bool flag);
  void setBlockReturn     (bool flag);

  //---

  // Alias
  Alias *defineAlias(const std::string &name, const std::string &value);

  void undefineAlias(const std::string &name);

  Alias *lookupAlias(const std::string &name) const;

  bool substituteAlias(Cmd *cmd, CmdArray &cmds) const;

  void displayAliases(bool alias) const;

  //---

  // AutoExec

  void defineAutoExec(const std::string &name, const std::string &value);

  void undefineAutoExec(const std::string &name);

  AutoExec *lookupAutoExec(const std::string &name) const;

  void displayAutoExec() const;

  //---

  // History

  int getHistoryCommandNum() const;

  bool findHistoryCommandStart(const std::string &text, int &commandNum);
  bool findHistoryCommandIn(const std::string &text, int &commandNum);
  bool findHistoryCommandArg(const std::string &text, int &commandNum, int &argNum);

  std::string getHistoryCommand(int num);
  std::string getHistoryCommandArg(int num, int argNum);

  void addHistoryFile(const std::string &filename);

  void addHistoryCommand(const std::string &text);

  void setHistoryCurrent(const std::string &text);

  void displayHistory(int num, bool showNumbers, bool showTime, bool reverse);

  bool hasPrevHistoryCommand();
  bool hasNextHistoryCommand();

  std::string getPrevHistoryCommand();
  std::string getNextHistoryCommand();

  //---

  // Shell Command

  ShellCommand *lookupShellCommand(const std::string &name) const;

  //---

  // Input

  void executeInput(const std::string &filename);

  void processInputLine(const Line &line);

  void getInputBlock(ShellCommand *command, LineArray &lines);
  void skipInputBlock(const Line &line);

  bool inputEof();
  Line getInputLine();

  std::string getInputPrompt();

  std::string processInputExprLine(const Line &line);

  //---

  // Read Line

  std::string readLine();
  void        beep();
  void        readInterrupt();

  //---

  // Dir Stack

  void pushDirStack();
  void pushDirStack(const std::string &dirname);

  std::string popDirStack();
  std::string popDirStack(int pos);

  int sizeDirStack();

  void printDirStack(bool expandHome=false);

  //---

  // Hash

  void        addFilePath(const std::string &filename, const std::string &path);
  std::string getFilePath(const std::string &filename);
  void        clearFilePath();
  void        printFilePathStats();
  void        setFilePathActive(bool flag);

  //---

  // Resource

  void limitResource(const std::string &name, const std::string &value, bool hard=false);
  void unlimitAllResources();
  void unlimitResource(const std::string &name);
  void printAllResources(bool hard=false);
  void printResource(const std::string &name, bool hard=false);

  //---

  void readTimeout();

  std::string colorLine(const std::string &line);

  std::string getAliasesMsg() const;
  std::string getHistoryMsg() const;

 private:
  void cleanup();

  void startup();

 private:
  using FileP = std::shared_ptr<CFile>;

  std::string commandString_;
  bool        exitOnError_    { false };
  bool        fastStartup_    { false };
  bool        interactive_    { false };
  bool        noExecute_      { false };
  bool        exitAfterCmd_   { false };
  bool        loginShell_     { false };
  std::string name_;
  std::string filename_;
  int         lineNum_        { -1 };
  PromptType  promptType_     { PromptType::NORMAL };
  std::string promptCommand_;
  bool        compatible_     { false };
  bool        silentMode_     { false };
  bool        debug_          { false };
  bool        interrupt_      { false };
  bool        verbose1_       { false };
  bool        verbose2_       { false };
  bool        echo1_          { false };
  bool        echo2_          { false };
  std::string argv0_;
  std::string initFilename_;
  FileP       inputFile_;
  int         termTries_      { 0 };
  int         exit_           { 0 };
  int         exitStatus_     { 0 };

  FunctionMgrP     functionMgr_;
  VariableMgrP     variableMgr_;
  ProcessMgrP      processMgr_;
  StateMgrP        stateMgr_;
  BlockMgrP        blockMgr_;
  AliasMgrP        aliasMgr_;
  AutoExecMgrP     autoExecMgr_;
  HistoryP         history_;
  ShellCommandMgrP shellCmdMgr_;
  InputP           input_;
  ReadLineP        readLine_;
  DirStackP        dirStack_;
  HashP            hash_;
  ResourceP        resource_;
  ServerP          server_;
#ifdef USE_SHM
  ShMemP           shMem_;
#endif
};

}

#endif
