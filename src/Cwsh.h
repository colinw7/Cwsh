#ifndef CWSH_H
#define CWSH_H

#define CwshMgrInst CwshMgr::getInstance()

#include <CAutoPtr.h>
#include <CAutoPtrMap.h>
#include <CAutoPtrVector.h>
#include <CAutoPtrStack.h>

class CFile;
class CMessage;

#ifdef USE_SHM
class CwshShMem;
#endif

enum CwshPromptType {
  CWSH_PROMPT_TYPE_NORMAL,
  CWSH_PROMPT_TYPE_EXTRA,
};

class CwshMgr {
 private:
  typedef std::list<Cwsh *> CwshList;

  CwshList cwsh_list_;

 public:
  static CwshMgr &getInstance();

 ~CwshMgr();

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
};

class Cwsh {
 private:
  std::string                   command_string_;
  bool                          exit_on_error_;
  bool                          fast_startup_;
  bool                          interactive_;
  bool                          no_execute_;
  bool                          exit_after_cmd_;
  bool                          login_shell_;
  std::string                   name_;
  std::string                   filename_;
  CwshPromptType                prompt_type_;
  std::string                   prompt_command_;
  bool                          compatible_;
  bool                          silentMode_;
  bool                          debug_;
  bool                          interrupt_;
  bool                          verbose1_;
  bool                          verbose2_;
  bool                          echo1_;
  bool                          echo2_;
  std::string                   argv0_;
  std::string                   init_filename_;
  CAutoPtr<CFile>               input_file_;
  int                           term_tries_;
  int                           exit_;
  int                           exit_status_;
  CAutoPtr<CwshFunctionMgr>     function_mgr_;
  CAutoPtr<CwshVariableMgr>     variable_mgr_;
  CAutoPtr<CwshProcessMgr>      process_mgr_;
  CAutoPtr<CwshStateMgr>        state_mgr_;
  CAutoPtr<CwshBlockMgr>        block_mgr_;
  CAutoPtr<CwshAliasMgr>        alias_mgr_;
  CAutoPtr<CwshAutoExecMgr>     auto_exec_mgr_;
  CAutoPtr<CwshHistory>         history_;
  CAutoPtr<CwshShellCommandMgr> shell_cmd_mgr_;
  CAutoPtr<CwshInput>           input_;
  CAutoPtr<CwshReadLine>        read_line_;
  CAutoPtr<CwshDirStack>        dir_stack_;
  CAutoPtr<CwshHash>            hash_;
  CAutoPtr<CwshResource>        resource_;
  CAutoPtr<CwshServer>          server_;
#ifdef USE_SHM
  CAutoPtr<CwshShMem>           sh_mem_;
#endif

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

  const std::string &getName         () const { return name_; }
  CwshPromptType     getPromptType   () const { return prompt_type_; }
  std::string        getPromptCommand() const { return prompt_command_; }
  bool               getCompatible   () const { return compatible_; }
  bool               getSilentMode   () const { return silentMode_; }
  bool               getDebug        () const { return debug_; }
  bool               getInterrupt    () const { return interrupt_; }
  std::string        getArgv0        () const { return argv0_; }

  CwshShellCommandMgr *getShellCommandMgr() const { return shell_cmd_mgr_; }

  bool changeDir(const std::string &dirname);

  void setFastStartup(bool flag=true) { fast_startup_ = flag; }

  void setPromptType(CwshPromptType prompt_type) {
    prompt_type_ = prompt_type;
  }

  void setPromptCommand(const std::string &prompt_command) {
    prompt_command_ = prompt_command;
  }

  void setInterrupt(bool flag) { interrupt_ = flag; }

  void setDebug(bool flag);

  void setExit(bool flag, int status=0) {
    exit_ = flag; exit_status_ = status;
  }

  bool getExit() const { return exit_; }

  // Function

  void defineFunction(const CwshFunctionName &name, const CwshLineArray &lines);
  void undefineFunction(const CwshFunctionName &name);

  CwshFunction *lookupFunction(const CwshFunctionName &name);

  void listAllFunctions();

  // Variable

  void defineVariable(const CwshVariableName &name);
  void defineVariable(const CwshVariableName &name,
                      const CwshVariableValue &value);
  void defineVariable(const CwshVariableName &name, int value);
  void defineVariable(const CwshVariableName &name,
                      const CwshVariableValueArray &values);
  void defineVariable(const CwshVariableName &name,
                      const char **values, int num_values);

  void undefineVariable(const CwshVariableName &name);

  CwshVariable *lookupVariable(const CwshVariableName &name) const;

  CwshVariableList::iterator variablesBegin();
  CwshVariableList::iterator variablesEnd();

  void listVariables() const;

  void saveVariables();
  void restoreVariables();

  bool isEnvironmentVariableLower(const std::string &name);
  bool isEnvironmentVariableUpper(const std::string &name);

  void updateEnvironmentVariable(CwshVariable *variable);

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

  // State

  void saveState();
  void restoreState();

  // Block

  void startBlock(CwshBlockType type, const CwshLineArray &lines);
  void endBlock();

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

  // Alias

  void defineAlias(const CwshAliasName &name, const CwshAliasValue &value);

  void undefineAlias(const CwshAliasName &name);

  CwshAlias *lookupAlias(const CwshAliasName &name) const;

  bool substituteAlias(CwshCmd *cmd, CwshCmdArray &cmds) const;

  void displayAlias() const;

  // AutoExec

  void defineAutoExec(const CwshAutoExecName &name, const CwshAutoExecValue &value);

  void undefineAutoExec(const CwshAutoExecName &name);

  CwshAutoExec *lookupAutoExec(const CwshAutoExecName &name) const;

  void displayAutoExec() const;

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

  // Shell Command

  CwshShellCommand *lookupShellCommand(const std::string &name) const;

  // Input

  void executeInput(const std::string &filename);

  void processInputLine(const CwshLine &line);

  void getInputBlock(CwshShellCommand *command, CwshLineArray &lines);
  void skipInputBlock(const CwshLine &line);

  bool     inputEof();
  CwshLine getInputLine();

  std::string getInputPrompt();

  std::string processInputExprLine(const CwshLine &line);

  // Read Line

  std::string readLine();
  void        beep();
  void        readInterrupt();

  // Dir Stack

  void pushDirStack();
  void pushDirStack(const std::string &dirname);

  std::string popDirStack();
  std::string popDirStack(int pos);

  int sizeDirStack();

  void printDirStack(bool expand_home=false);

  // Hash

  void        addFilePath(const std::string &filename, const std::string &path);
  std::string getFilePath(const std::string &filename);
  void        clearFilePath();
  void        printFilePathStats();
  void        setFilePathActive(bool flag);

  // Resource

  void limitResource(const std::string &name, const std::string &value,
                    bool hard=false);
  void unlimitAllResources();
  void unlimitResource(const std::string &name);
  void printAllResources(bool hard=false);
  void printResource(const std::string &name, bool hard=false);

  void readTimeout();

  std::string colorLine(const std::string &line);

  std::string getAliasesMsg() const;
  std::string getHistoryMsg() const;

 private:
  void cleanup();

  void startup();
};

#endif
