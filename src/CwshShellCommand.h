#ifndef CWSH_SHELL_COMMAND_H
#define CWSH_SHELL_COMMAND_H

#define CWSH_SHELL_COMMAND_FLAGS_NONE         0
#define CWSH_SHELL_COMMAND_FLAGS_NO_WILDCARDS (1<<0)
#define CWSH_SHELL_COMMAND_FLAGS_NO_EXPAND    (1<<1)

struct CwshShellCommandData {
  const char *          name_;
  const char *          end_name_;
  CwshShellCommandProc  proc_;
  int                   flags_;
};

class CwshShellCommand {
 public:
  CwshShellCommand(Cwsh *cwsh, CwshShellCommandData *data) :
   cwsh_(cwsh), data_(data) {
  }

  Cwsh *getCwsh() const { return cwsh_; }

  const char *getName   () const { return data_->name_; }
  const char *getEndName() const { return data_->end_name_; }

  CwshShellCommandProc getProc() const { return data_->proc_; }

  bool getNoExpand() const {
    return (data_->flags_ & CWSH_SHELL_COMMAND_FLAGS_NO_EXPAND);
  }

  bool getNoWildcards() const {
    return ((data_->flags_ & CWSH_SHELL_COMMAND_FLAGS_NO_WILDCARDS) ||
            (data_->flags_ & CWSH_SHELL_COMMAND_FLAGS_NO_EXPAND   ));
  }

  bool isBlockCommand() const { return (data_->end_name_ != NULL); }

 private:
  CPtr<Cwsh>            cwsh_;
  CwshShellCommandData *data_;
};

//---

class CwshShellCommandMgr {
 public:
  CwshShellCommandMgr(Cwsh *cwsh);
 ~CwshShellCommandMgr();

  CwshShellCommand *lookup(const std::string &name) const;

  static void runProc(const CwshArgArray &args, CCommand::CallbackData data);

  static void colonCmd   (Cwsh *cwsh, const CwshArgArray &args);
  static void aliasCmd   (Cwsh *cwsh, const CwshArgArray &args);
  static void autoExecCmd(Cwsh *cwsh, const CwshArgArray &args);
  static void bgCmd      (Cwsh *cwsh, const CwshArgArray &args);
  static void breakCmd   (Cwsh *cwsh, const CwshArgArray &args);
  static void breakswCmd (Cwsh *cwsh, const CwshArgArray &args);
  static void caseCmd    (Cwsh *cwsh, const CwshArgArray &args);
  static void cdCmd      (Cwsh *cwsh, const CwshArgArray &args);
  static void completeCmd(Cwsh *cwsh, const CwshArgArray &args);
  static void continueCmd(Cwsh *cwsh, const CwshArgArray &args);
  static void defaultCmd (Cwsh *cwsh, const CwshArgArray &args);
  static void dirsCmd    (Cwsh *cwsh, const CwshArgArray &args);
  static void echoCmd    (Cwsh *cwsh, const CwshArgArray &args);
  static void elseCmd    (Cwsh *cwsh, const CwshArgArray &args);
  static void endCmd     (Cwsh *cwsh, const CwshArgArray &args);
  static void endfuncCmd (Cwsh *cwsh, const CwshArgArray &args);
  static void endifCmd   (Cwsh *cwsh, const CwshArgArray &args);
  static void endswCmd   (Cwsh *cwsh, const CwshArgArray &args);
  static void evalCmd    (Cwsh *cwsh, const CwshArgArray &args);
  static void exprCmd    (Cwsh *cwsh, const CwshArgArray &args);
  static void execCmd    (Cwsh *cwsh, const CwshArgArray &args);
  static void exitCmd    (Cwsh *cwsh, const CwshArgArray &args);
  static void fgCmd      (Cwsh *cwsh, const CwshArgArray &args);
  static void foreachCmd (Cwsh *cwsh, const CwshArgArray &args);
  static void funcCmd    (Cwsh *cwsh, const CwshArgArray &args);
  static void globCmd    (Cwsh *cwsh, const CwshArgArray &args);
  static void gotoCmd    (Cwsh *cwsh, const CwshArgArray &args);
  static void hashstatCmd(Cwsh *cwsh, const CwshArgArray &args);
  static void helpCmd    (Cwsh *cwsh, const CwshArgArray &args);
  static void historyCmd (Cwsh *cwsh, const CwshArgArray &args);
  static void ifCmd      (Cwsh *cwsh, const CwshArgArray &args);
  static void jobsCmd    (Cwsh *cwsh, const CwshArgArray &args);
  static void killCmd    (Cwsh *cwsh, const CwshArgArray &args);
  static void limitCmd   (Cwsh *cwsh, const CwshArgArray &args);
  static void niceCmd    (Cwsh *cwsh, const CwshArgArray &args);
  static void nohupCmd   (Cwsh *cwsh, const CwshArgArray &args);
  static void notifyCmd  (Cwsh *cwsh, const CwshArgArray &args);
  static void onintrCmd  (Cwsh *cwsh, const CwshArgArray &args);
  static void popdCmd    (Cwsh *cwsh, const CwshArgArray &args);
  static void printenvCmd(Cwsh *cwsh, const CwshArgArray &args);
  static void pushdCmd   (Cwsh *cwsh, const CwshArgArray &args);
  static void rehashCmd  (Cwsh *cwsh, const CwshArgArray &args);
  static void repeatCmd  (Cwsh *cwsh, const CwshArgArray &args);
  static void returnCmd  (Cwsh *cwsh, const CwshArgArray &args);
  static void setCmd     (Cwsh *cwsh, const CwshArgArray &args);
  static void setenvCmd  (Cwsh *cwsh, const CwshArgArray &args);
  static void shiftCmd   (Cwsh *cwsh, const CwshArgArray &args);
  static void sourceCmd  (Cwsh *cwsh, const CwshArgArray &args);
  static void stopCmd    (Cwsh *cwsh, const CwshArgArray &args);
  static void suspendCmd (Cwsh *cwsh, const CwshArgArray &args);
  static void switchCmd  (Cwsh *cwsh, const CwshArgArray &args);
  static void timeCmd    (Cwsh *cwsh, const CwshArgArray &args);
  static void umaskCmd   (Cwsh *cwsh, const CwshArgArray &args);
  static void unaliasCmd (Cwsh *cwsh, const CwshArgArray &args);
  static void unhashCmd  (Cwsh *cwsh, const CwshArgArray &args);
  static void unlimitCmd (Cwsh *cwsh, const CwshArgArray &args);
  static void unsetCmd   (Cwsh *cwsh, const CwshArgArray &args);
  static void unsetenvCmd(Cwsh *cwsh, const CwshArgArray &args);
  static void waitCmd    (Cwsh *cwsh, const CwshArgArray &args);
  static void whichCmd   (Cwsh *cwsh, const CwshArgArray &args);
  static void whileCmd   (Cwsh *cwsh, const CwshArgArray &args);
  static void atCmd      (Cwsh *cwsh, const CwshArgArray &args);
  static void badCmd     (Cwsh *cwsh, const CwshArgArray &args);

  static bool isHelpArg(const CwshArgArray &args);

 private:
  static CwshShellCommandData commands_data_[];

  CPtr<Cwsh>                      cwsh_;
  std::vector<CwshShellCommand *> commands_;
};

#endif
