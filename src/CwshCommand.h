#ifndef CWSH_COMMAND_H
#define CWSH_COMMAND_H

#include <CCommand.h>

namespace Cwsh {

enum class CommandType {
  SUBSHELL,
  PROCESS,
  LABEL,
  FUNCTION,
  SHELL,
  UNIX
};

namespace CommandUtil {
  bool parseCommandLines (App *cwsh, const std::string &str, CmdLineArray &cmds);
  bool parseCommandGroups(App *cwsh, const std::string &str, CmdGroupArray &groups);

  bool groupCommands(CmdArray cmds, CmdGroupArray &groups);

  CmdArray parseCommandGroup(App *cwsh, CmdGroup *group);

  CommandType getType(App *cwsh, const std::vector<std::string> &words);

  void processLineProc(const ArgArray &args, CCommand::CallbackData data);
}

//---

class Command : public CCommand {
 public:
  Command(App *cwsh, const std::string &name, const std::string &path,
          const StringVectorT &args=StringVectorT(), bool doFork=true);
  Command(App *cwsh, const std::string &name, CallbackProc proc, CallbackData data,
          const StringVectorT &args=StringVectorT(), bool doFork=false);

 ~Command();

  void setNotify(bool notify) { notify_ = notify; }

  void setState(State state) override;

 private:
  CPtr<App> cwsh_;
  bool      notify_       { false };
  bool      stateChanged_ { false };
};

//---

class CommandData {
 public:
  CommandData(App *cwsh, const std::vector<std::string> &words);
 ~CommandData();

  CommandType  getType   () const { return type_; }
  Command     *getCommand() const;

 private:
  using CommandP = std::shared_ptr<Command>;

  CPtr<App>   cwsh_;
  CommandType type_;
  CommandP    command_;
};

}

#endif
