#ifndef CWSH_H
#define CWSH_H

#define CwshMgrInst CwshMgr::getInstance()

#include <CConfig.h>
#include <CAutoPtr.h>
#include <CAutoPtrMap.h>
#include <CAutoPtrVector.h>
#include <CAutoPtrStack.h>

class CFile;
class CMessage;

#ifdef USE_SHM
class CwshShMem;
#endif

enum class CwshPromptType {
  NORMAL,
  EXTRA
};

//---

class CwshMgr {
 private:
  typedef std::list<Cwsh *> CwshList;

 public:
  static CwshMgr &getInstance();

 ~CwshMgr();

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

  void add   (Cwsh *cwsh);
  void remove(Cwsh *cwsh);

  void term(int status);
  void term(Cwsh *cwsh, int status);

  void setInterrupt(bool flag);
  void readInterrupt();

  void gotoBlockLabel(const std::string &label);

  void stopActiveProcesses();

 private:
  CwshMgr();

 private:
  CConfig     config_;
  CwshList    cwshList_;
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

class Cwsh {
 public:
  Cwsh();
 ~Cwsh();

  void init();
  void init(int argc, char **argv);
  void term();

  bool processArgs(int argc, char **argv);
  void initEnv();

  void enableServer();

  static CMessage *createServerMessage();

  void mainLoop();
  void processLine(const std::string &str);

  //---

  const std::string &getName         () const { return name_; }
  bool               getCompatible   () const { return compatible_; }
  bool               getSilentMode   () const { return silentMode_; }
  std::string        getArgv0        () const { return argv0_; }

  CwshShellCommandMgr *getShellCommandMgr() const { return shellCmdMgr_; }

  bool changeDir(const std::string &dirname);

  void setFastStartup(bool flag=true) { fastStartup_ = flag; }

  // get/set filename
  void setFilename(const std::string &name) { filename_ = name; }
  const std::string &getFilename() const { return filename_; }

  // get/set line number
  void setLineNum(int num) { lineNum_ = num; }
  int getLineNum() const { return lineNum_; }

  // get/set prompt type
  CwshPromptType getPromptType() const { return promptType_; }
  void setPromptType(CwshPromptType type) { promptType_ = type; }

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

  CwshFunction *defineFunction(const CwshFunctionName &name, const CwshLineArray &lines);

  void undefineFunction(const CwshFunctionName &name);

  CwshFunction *lookupFunction(const CwshFunctionName &name);

  void listAllFunctions();

  //---

  // Variable

  CwshVariable *defineVariable(const CwshVariableName &name);
  CwshVariable *defineVariable(const CwshVariableName &name, const CwshVariableValue &value);
  CwshVariable *defineVariable(const CwshVariableName &name, int value);
  CwshVariable *defineVariable(const CwshVariableName &name, const CwshVariableValueArray &values);
  CwshVariable *defineVariable(const CwshVariableName &name, const char **values, int num_values);

  void undefineVariable(const CwshVariableName &name);

  CwshVariable *lookupVariable(const CwshVariableName &name) const;

  CwshVariableList::iterator variablesBegin();
  CwshVariableList::iterator variablesEnd();

  void listVariables(bool all) const;

  void saveVariables();
  void restoreVariables();

  bool isEnvironmentVariableLower(const std::string &name);
  bool isEnvironmentVariableUpper(const std::string &name);

  void updateEnvironmentVariable(CwshVariable *variable);

  //---

  // Process

  CwshProcess *addProcess(CwshCommandData *command);

  void removeProcess(CwshProcess *process);

  void killProcess(int pid, int signal);

  int getNumActiveProcesses();

  void displayActiveProcesses(bool list_pids);
  void displayExitedProcesses();

  void waitActiveProcesses();

  int stringToProcessId(const std::string &str);

  CwshProcess *getActiveProcess(const std::string &str);
  CwshProcess *getCurrentActiveProcess();

  CwshProcess *lookupProcess(pid_t pid);

  //---

  // State
  void saveState();
  void restoreState();

  //---

  // Block
  CwshBlock *startBlock(CwshBlockType type, const CwshLineArray &lines);
  void       endBlock();

  bool     inBlock      () const;
  bool     blockEof     () const;
  CwshLine blockReadLine() const;

  CwshBlock *findBlock(CwshBlockType type);

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
  CwshAlias *defineAlias(const CwshAliasName &name, const CwshAliasValue &value);

  void undefineAlias(const CwshAliasName &name);

  CwshAlias *lookupAlias(const CwshAliasName &name) const;

  bool substituteAlias(CwshCmd *cmd, CwshCmdArray &cmds) const;

  void displayAliases(bool alias) const;

  //---

  // AutoExec

  void defineAutoExec(const CwshAutoExecName &name, const CwshAutoExecValue &value);

  void undefineAutoExec(const CwshAutoExecName &name);

  CwshAutoExec *lookupAutoExec(const CwshAutoExecName &name) const;

  void displayAutoExec() const;

  //---

  // History

  int getHistoryCommandNum() const;

  bool findHistoryCommandStart(const std::string &text, int &command_num);
  bool findHistoryCommandIn(const std::string &text, int &command_num);
  bool findHistoryCommandArg(const std::string &text, int &command_num,
                             int &arg_num);

  std::string getHistoryCommand(int num);
  std::string getHistoryCommandArg(int num, int arg_num);

  void addHistoryFile(const std::string &filename);

  void addHistoryCommand(const std::string &text);

  void setHistoryCurrent(const std::string &text);

  void displayHistory(int num, bool show_numbers,
                      bool show_time, bool reverse);

  bool hasPrevHistoryCommand();
  bool hasNextHistoryCommand();

  std::string getPrevHistoryCommand();
  std::string getNextHistoryCommand();

  //---

  // Shell Command

  CwshShellCommand *lookupShellCommand(const std::string &name) const;

  //---

  // Input

  void executeInput(const std::string &filename);

  void processInputLine(const CwshLine &line);

  void getInputBlock(CwshShellCommand *command, CwshLineArray &lines);
  void skipInputBlock(const CwshLine &line);

  bool     inputEof();
  CwshLine getInputLine();

  std::string getInputPrompt();

  std::string processInputExprLine(const CwshLine &line);

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

  void printDirStack(bool expand_home=false);

  //---

  // Hash

  void        addFilePath(const std::string &filename, const std::string &path);
  std::string getFilePath(const std::string &filename);
  void        clearFilePath();
  void        printFilePathStats();
  void        setFilePathActive(bool flag);

  //---

  // Resource

  void limitResource(const std::string &name, const std::string &value,
                    bool hard=false);
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
  std::string                   command_string_;
  bool                          exitOnError_    { false };
  bool                          fastStartup_    { false };
  bool                          interactive_    { false };
  bool                          noExecute_      { false };
  bool                          exitAfterCmd_   { false };
  bool                          loginShell_     { false };
  std::string                   name_;
  std::string                   filename_;
  int                           lineNum_        { -1 };
  CwshPromptType                promptType_     { CwshPromptType::NORMAL };
  std::string                   promptCommand_;
  bool                          compatible_     { false };
  bool                          silentMode_     { false };
  bool                          debug_          { false };
  bool                          interrupt_      { false };
  bool                          verbose1_       { false };
  bool                          verbose2_       { false };
  bool                          echo1_          { false };
  bool                          echo2_          { false };
  std::string                   argv0_;
  std::string                   initFilename_;
  CAutoPtr<CFile>               inputFile_;
  int                           termTries_      { 0 };
  int                           exit_           { 0 };
  int                           exitStatus_     { 0 };
  CAutoPtr<CwshFunctionMgr>     functionMgr_;
  CAutoPtr<CwshVariableMgr>     variableMgr_;
  CAutoPtr<CwshProcessMgr>      processMgr_;
  CAutoPtr<CwshStateMgr>        stateMgr_;
  CAutoPtr<CwshBlockMgr>        blockMgr_;
  CAutoPtr<CwshAliasMgr>        aliasMgr_;
  CAutoPtr<CwshAutoExecMgr>     autoExecMgr_;
  CAutoPtr<CwshHistory>         history_;
  CAutoPtr<CwshShellCommandMgr> shellCmdMgr_;
  CAutoPtr<CwshInput>           input_;
  CAutoPtr<CwshReadLine>        readLine_;
  CAutoPtr<CwshDirStack>        dirStack_;
  CAutoPtr<CwshHash>            hash_;
  CAutoPtr<CwshResource>        resource_;
  CAutoPtr<CwshServer>          server_;
#ifdef USE_SHM
  CAutoPtr<CwshShMem>           shMem_;
#endif
};

#endif
