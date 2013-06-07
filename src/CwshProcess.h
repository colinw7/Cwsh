#ifndef CWSH_PROCESS_H
#define CWSH_PROCESS_H

enum CwshProcessMatchType {
  CWSH_PROCESS_MATCH_START,
  CWSH_PROCESS_MATCH_ANY,
};

class CwshProcessMgr {
 public:
  CwshProcessMgr(Cwsh *cwsh);

  CwshProcess *add(CwshCommandData *command);

  void remove(CwshProcess *process);

  void kill(pid_t pid, int signal);

  int  getNumActive();
  void displayActive(bool list_pids);
  void displayExited();
  void deleteExited();

  void waitActive();

  pid_t stringToPid(const std::string &str);

  CwshProcess *getActiveProcess(const std::string &str);
  CwshProcess *getCurrentActiveProcess();
  CwshProcess *getPreviousActiveProcess();
  CwshProcess *getActiveProcess(int num);
  CwshProcess *matchActiveProcess(const std::string &str, CwshProcessMatchType match_type);

  CwshProcess *lookupProcess(pid_t pid);

 private:
  typedef std::list<CwshProcess *> ProcessList;

  CPtr<Cwsh>  cwsh_;
  ProcessList processes_;
};

class CwshProcess {
  CINST_COUNT_MEMBER(CwshProcess);

 private:
  typedef std::vector<CwshCommandData *> SubCommands;

 public:
  CwshProcess(CwshCommandData *command, int num=1);
 ~CwshProcess();

  CwshCommandData *getCommand() const { return command_; }

  int getNum() const { return num_; }

  CCommand::State getCommandState () const;
  std::string     getCommandString() const;
  pid_t           getCommandPid() const;

  void addSubCommand(CwshCommandData *command);

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
  CwshCommandData *command_;
  int              num_;
  SubCommands      subCommands_;
};

#endif
