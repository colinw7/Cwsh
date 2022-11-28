#ifndef CWSH_PROCESS_H
#define CWSH_PROCESS_H

namespace Cwsh {

enum class ProcessMatchType {
  START,
  ANY,
};

class ProcessMgr {
 public:
  using ProcessP = std::shared_ptr<Process>;

 public:
  ProcessMgr(App *cwsh);

  Process *add(CommandData *command);

  void remove(Process *process);

  void kill(pid_t pid, int signal);

  int  getNumActive();
  void displayActive(bool listPids);
  void displayExited();
  void deleteExited();

  void waitActive();

  pid_t stringToPid(const std::string &str);

  Process *getActiveProcess(const std::string &str);
  Process *getCurrentActiveProcess();
  Process *getPreviousActiveProcess();
  Process *getActiveProcess(int num);
  Process *matchActiveProcess(const std::string &str, ProcessMatchType matchType);

  Process *lookupProcess(pid_t pid);

 private:
  void remove(ProcessP process);

 private:
  using ProcessList = std::list<ProcessP>;

  CPtr<App>   cwsh_;
  ProcessList processes_;
};

//---

class Process {
  CINST_COUNT_MEMBER(Process);

 private:
  using SubCommands = std::vector<CommandData *>;

 public:
  Process(CommandData *command, int num=1);
 ~Process();

  CommandData *getCommand() const { return command_; }

  int getNum() const { return num_; }

  CCommand::State getCommandState () const;
  std::string     getCommandString() const;
  pid_t           getCommandPid() const;

  void addSubCommand(CommandData *command);

  const SubCommands &getSubCommands() const { return subCommands_; }

  void start();
  void stop();
  void tstop();
  void pause();
  void resume();
  void wait();

  void setNotify(bool flag);

  bool isPid(pid_t pid) const;

  void print() const;

 private:
  CommandData *command_;
  int          num_;
  SubCommands  subCommands_;
};

}

#endif
