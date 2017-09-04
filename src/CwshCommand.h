#ifndef CWSH_COMMAND_H
#define CWSH_COMMAND_H

enum class CwshCommandType {
  SUBSHELL,
  PROCESS,
  LABEL,
  FUNCTION,
  SHELL,
  UNIX
};

#include <CCommand.h>

namespace CwshCommandUtil {
  bool parseCommandLines (Cwsh *cwsh, const std::string &str, CwshCmdLineArray &cmds);
  bool parseCommandGroups(Cwsh *cwsh, const std::string &str, CwshCmdGroupArray &groups);

  bool groupCommands(CwshCmdArray cmds, CwshCmdGroupArray &groups);

  CwshCmdArray parseCommandGroup(Cwsh *cwsh, CwshCmdGroup *group);

  CwshCommandType getType(Cwsh *cwsh, const std::vector<std::string> &words);

  void processLineProc(const CwshArgArray &args, CCommand::CallbackData data);
}

//---

class CwshCommand : public CCommand {
 public:
  CwshCommand(Cwsh *cwsh, const std::string &name, const std::string &path,
              const StringVectorT &args=StringVectorT(), bool do_fork=true);
  CwshCommand(Cwsh *cwsh, const std::string &name, CallbackProc proc, CallbackData data,
              const StringVectorT &args=StringVectorT(), bool do_fork=false);

 ~CwshCommand();

  void setNotify(bool notify) { notify_ = notify; }

  void setState(State state);

 private:
  CPtr<Cwsh> cwsh_;
  bool       notify_       { false };
  bool       stateChanged_ { false };
};

//---

class CwshCommandData {
 public:
  CwshCommandData(Cwsh *cwsh, const std::vector<std::string> &words);
 ~CwshCommandData();

  CwshCommandType  getType   () const { return type_; }
  CwshCommand     *getCommand() const { return command_; }

 private:
  CPtr<Cwsh>            cwsh_;
  CwshCommandType       type_;
  CAutoPtr<CwshCommand> command_;
};

#endif
