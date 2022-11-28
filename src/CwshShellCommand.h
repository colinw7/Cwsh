#ifndef CWSH_SHELL_COMMAND_H
#define CWSH_SHELL_COMMAND_H

#define CWSH_SHELL_COMMAND_FLAGS_NONE         0
#define CWSH_SHELL_COMMAND_FLAGS_NO_WILDCARDS (1<<0)
#define CWSH_SHELL_COMMAND_FLAGS_NO_EXPAND    (1<<1)

namespace Cwsh {

using ShellCommandProc = void (*)(App *cwsh, const ArgArray &args);

struct ShellCommandData {
  const char *     name_;
  const char *     endName_;
  ShellCommandProc proc_;
  int              flags_;
};

class ShellCommand {
 public:
  ShellCommand(App *cwsh, ShellCommandData *data) :
   cwsh_(cwsh), data_(data) {
  }

  App *getCwsh() const { return cwsh_; }

  const char *getName   () const { return data_->name_; }
  const char *getEndName() const { return data_->endName_; }

  ShellCommandProc getProc() const { return data_->proc_; }

  bool getNoExpand() const {
    return (data_->flags_ & CWSH_SHELL_COMMAND_FLAGS_NO_EXPAND);
  }

  bool getNoWildcards() const {
    return ((data_->flags_ & CWSH_SHELL_COMMAND_FLAGS_NO_WILDCARDS) ||
            (data_->flags_ & CWSH_SHELL_COMMAND_FLAGS_NO_EXPAND   ));
  }

  bool isBlockCommand() const { return (data_->endName_ != nullptr); }

 private:
  CPtr<App>         cwsh_;
  ShellCommandData *data_;
};

//---

class ShellCommandMgr {
 public:
  ShellCommandMgr(App *cwsh);
 ~ShellCommandMgr();

  ShellCommand *lookup(const std::string &name) const;

  static void runProc(const ArgArray &args, CCommand::CallbackData data);

  static void colonCmd   (App *cwsh, const ArgArray &args);
  static void aliasCmd   (App *cwsh, const ArgArray &args);
  static void autoExecCmd(App *cwsh, const ArgArray &args);
  static void bgCmd      (App *cwsh, const ArgArray &args);
  static void breakCmd   (App *cwsh, const ArgArray &args);
  static void breakswCmd (App *cwsh, const ArgArray &args);
  static void caseCmd    (App *cwsh, const ArgArray &args);
  static void cdCmd      (App *cwsh, const ArgArray &args);
  static void completeCmd(App *cwsh, const ArgArray &args);
  static void continueCmd(App *cwsh, const ArgArray &args);
  static void defaultCmd (App *cwsh, const ArgArray &args);
  static void dirsCmd    (App *cwsh, const ArgArray &args);
  static void echoCmd    (App *cwsh, const ArgArray &args);
  static void elseCmd    (App *cwsh, const ArgArray &args);
  static void endCmd     (App *cwsh, const ArgArray &args);
  static void endfuncCmd (App *cwsh, const ArgArray &args);
  static void endifCmd   (App *cwsh, const ArgArray &args);
  static void endswCmd   (App *cwsh, const ArgArray &args);
  static void evalCmd    (App *cwsh, const ArgArray &args);
  static void exprCmd    (App *cwsh, const ArgArray &args);
  static void execCmd    (App *cwsh, const ArgArray &args);
  static void exitCmd    (App *cwsh, const ArgArray &args);
  static void fgCmd      (App *cwsh, const ArgArray &args);
  static void foreachCmd (App *cwsh, const ArgArray &args);
  static void funcCmd    (App *cwsh, const ArgArray &args);
  static void globCmd    (App *cwsh, const ArgArray &args);
  static void gotoCmd    (App *cwsh, const ArgArray &args);
  static void hashstatCmd(App *cwsh, const ArgArray &args);
  static void helpCmd    (App *cwsh, const ArgArray &args);
  static void historyCmd (App *cwsh, const ArgArray &args);
  static void ifCmd      (App *cwsh, const ArgArray &args);
  static void jobsCmd    (App *cwsh, const ArgArray &args);
  static void killCmd    (App *cwsh, const ArgArray &args);
  static void limitCmd   (App *cwsh, const ArgArray &args);
  static void niceCmd    (App *cwsh, const ArgArray &args);
  static void nohupCmd   (App *cwsh, const ArgArray &args);
  static void notifyCmd  (App *cwsh, const ArgArray &args);
  static void onintrCmd  (App *cwsh, const ArgArray &args);
  static void popdCmd    (App *cwsh, const ArgArray &args);
  static void printenvCmd(App *cwsh, const ArgArray &args);
  static void pushdCmd   (App *cwsh, const ArgArray &args);
  static void rehashCmd  (App *cwsh, const ArgArray &args);
  static void repeatCmd  (App *cwsh, const ArgArray &args);
  static void returnCmd  (App *cwsh, const ArgArray &args);
  static void setCmd     (App *cwsh, const ArgArray &args);
  static void setenvCmd  (App *cwsh, const ArgArray &args);
  static void shiftCmd   (App *cwsh, const ArgArray &args);
  static void sourceCmd  (App *cwsh, const ArgArray &args);
  static void stopCmd    (App *cwsh, const ArgArray &args);
  static void suspendCmd (App *cwsh, const ArgArray &args);
  static void switchCmd  (App *cwsh, const ArgArray &args);
  static void timeCmd    (App *cwsh, const ArgArray &args);
  static void umaskCmd   (App *cwsh, const ArgArray &args);
  static void unaliasCmd (App *cwsh, const ArgArray &args);
  static void unhashCmd  (App *cwsh, const ArgArray &args);
  static void unlimitCmd (App *cwsh, const ArgArray &args);
  static void unsetCmd   (App *cwsh, const ArgArray &args);
  static void unsetenvCmd(App *cwsh, const ArgArray &args);
  static void waitCmd    (App *cwsh, const ArgArray &args);
  static void whichCmd   (App *cwsh, const ArgArray &args);
  static void whileCmd   (App *cwsh, const ArgArray &args);
  static void atCmd      (App *cwsh, const ArgArray &args);
  static void badCmd     (App *cwsh, const ArgArray &args);

  static bool isHelpArg(const ArgArray &args);

 private:
  static ShellCommandData commandsData_[];

  CPtr<App>                   cwsh_;
  std::vector<ShellCommand *> commands_;
};

}

#endif
