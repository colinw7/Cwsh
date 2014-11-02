#include <CwshI.h>
#include <COSProcess.h>
#include <cstring>
#include <cerrno>

CwshProcessMgr::
CwshProcessMgr(Cwsh *cwsh) :
 cwsh_(cwsh)
{
}

CwshProcess *
CwshProcessMgr::
add(CwshCommandData *command)
{
  int num = 1;

  if (processes_.size() > 0)
    num = processes_.back()->getNum() + 1;

  CwshProcess *process = new CwshProcess(command, num);

  processes_.push_back(process);

  return process;
}

void
CwshProcessMgr::
remove(CwshProcess *process)
{
  processes_.remove(process);

  delete process;
}

void
CwshProcessMgr::
kill(pid_t pid, int signal)
{
  CwshProcess *process = lookupProcess(pid);

  if (process == NULL) {
    int error = COSProcess::killProcess(pid, signal);

    if (error < 0)
      CWSH_THROW("No such process");

    return;
  }

  CCommand::State state = process->getCommandState();

  if (state == CCommand::RUNNING_STATE || state == CCommand::STOPPED_STATE) {
    int error_code = COSProcess::killProcess(pid, signal);

    if (error_code != 0)
      CWSH_THROW(string("kill: ") + strerror(errno) + ".");
  }
}

int
CwshProcessMgr::
getNumActive()
{
  int count  = 0;

  ProcessList::iterator p1 = processes_.begin();
  ProcessList::iterator p2 = processes_.end  ();

  for ( ; p1 != p2; ++p1) {
    CCommand::State state = (*p1)->getCommandState();

    if (state != CCommand::RUNNING_STATE && state != CCommand::STOPPED_STATE)
      continue;

    count++;
  }

  return count;
}

void
CwshProcessMgr::
displayActive(bool list_pids)
{
  vector<CwshProcess *> active_processes;

  ProcessList::iterator p1 = processes_.begin();
  ProcessList::iterator p2 = processes_.end  ();

  for ( ; p1 != p2; ++p1) {
    CCommand::State state = (*p1)->getCommandState();

    if (state == CCommand::EXITED_STATE)
      continue;

    if (! (*p1)->getCommand()->getCommand()->getDoFork())
      continue;

    active_processes.push_back(*p1);
  }

  uint num_active_processes = active_processes.size();

  for (uint i = 0; i < num_active_processes; i++) {
    CwshProcess *process = active_processes[i];

    std::cout << "[" << process->getNum() << "]  ";

    if      (i == num_active_processes - 1)
      std::cout << "+ ";
    else if (i == num_active_processes - 2)
      std::cout << "- ";
    else
      std::cout << "  ";

    if (list_pids)
      std::cout << " " << process->getCommandPid() << " ";

    if      (process->getCommandState() == CCommand::STOPPED_STATE)
      std::cout << "Suspended             ";
    else
      std::cout << "Running               ";

    process->print();

    std::cout << std::endl;
  }

  deleteExited();
}

void
CwshProcessMgr::
displayExited()
{
  int count = 0;

  ProcessList::iterator p1 = processes_.begin();
  ProcessList::iterator p2 = processes_.end  ();

  while (p1 != p2) {
    CwshProcess *process = *p1;

    CCommand::State state = process->getCommandState();

    if (state == CCommand::RUNNING_STATE || state == CCommand::STOPPED_STATE ||
        state == CCommand::EXITED_STATE)
      count++;

    if (state == CCommand::EXITED_STATE) {
      std::cout << "[" << process->getNum() << "]    Done                  ";

      process->print();

      std::cout << std::endl;

      remove(process);

      p1 = processes_.begin();
    }
    else
      ++p1;
  }
}

void
CwshProcessMgr::
deleteExited()
{
  ProcessList::iterator p1, p2;

  for (p1 = processes_.begin(), p2 = processes_.end (); p1 != p2; ++p1) {
    CwshProcess *process = *p1;

    CCommand::State state = process->getCommandState();

    if (state == CCommand::EXITED_STATE)
      remove(process);
  }
}

void
CwshProcessMgr::
waitActive()
{
  ProcessList::iterator p1, p2;

  for (p1 = processes_.begin(), p2 = processes_.end (); p1 != p2; ++p1) {
    CwshProcess *process = *p1;

    CwshCommand *command = process->getCommand()->getCommand();

    if (! command->getDoFork())
      continue;

    command->wait();
  }

  displayExited();
}

pid_t
CwshProcessMgr::
stringToPid(const string &str)
{
  CwshProcess *process = getActiveProcess(str);

  return process->getCommandPid();
}

CwshProcess *
CwshProcessMgr::
getActiveProcess(const string &str)
{
  if (str.size() < 1 || str[0] != '%')
    CWSH_THROW("No current job.");

  CwshProcess *process = NULL;

  if      (str.size() == 1 ||
           (str.size() == 2 && (str[1] == '%' || str[1] == '+')))
    process = getCurrentActiveProcess();
  else if (str.size() == 2 && str[1] == '-')
    process = getPreviousActiveProcess();
  else if (str.size() > 1 && CStrUtil::isInteger(str.substr(1))) {
    int process_num = CStrUtil::toInteger(str.substr(1));

    process = getActiveProcess(process_num);
  }
  else if (str[1] == '?')
    process = matchActiveProcess(str.substr(1), CWSH_PROCESS_MATCH_ANY);
  else
    process = matchActiveProcess(str.substr(1), CWSH_PROCESS_MATCH_START);

  if (process == NULL)
    CWSH_THROW("No such job.");

  return process;
}

CwshProcess *
CwshProcessMgr::
getCurrentActiveProcess()
{
  CwshProcess *process = NULL;

  ProcessList::iterator p1 = processes_.begin();
  ProcessList::iterator p2 = processes_.end  ();

  for ( ; p1 != p2; ++p1) {
    CwshCommand *command = (*p1)->getCommand()->getCommand();

    CCommand::State state = command->getState();

    if (state != CCommand::RUNNING_STATE && state != CCommand::STOPPED_STATE)
      continue;

    process = *p1;
  }

  return process;
}

CwshProcess *
CwshProcessMgr::
getPreviousActiveProcess()
{
  CwshProcess *process1 = NULL;
  CwshProcess *process2 = NULL;

  ProcessList::iterator p1 = processes_.begin();
  ProcessList::iterator p2 = processes_.end  ();

  for ( ; p1 != p2; ++p1) {
    CwshCommand *command = (*p1)->getCommand()->getCommand();

    CCommand::State state = command->getState();

    if (state != CCommand::RUNNING_STATE && state != CCommand::STOPPED_STATE)
      continue;

    process1 = process2;
    process2 = *p1;
  }

  return process1;
}

CwshProcess *
CwshProcessMgr::
getActiveProcess(int num)
{
  vector<CwshProcess *> active_processes;

  ProcessList::iterator p1 = processes_.begin();
  ProcessList::iterator p2 = processes_.end  ();

  for ( ; p1 != p2; ++p1) {
    CwshCommand *command = (*p1)->getCommand()->getCommand();

    CCommand::State state = command->getState();

    if (state != CCommand::RUNNING_STATE && state != CCommand::STOPPED_STATE)
      continue;

    active_processes.push_back(*p1);
  }

  int num_active_processes = active_processes.size();

  if (num > num_active_processes)
    return NULL;

  return active_processes[num_active_processes - num];
}

CwshProcess *
CwshProcessMgr::
matchActiveProcess(const string &str, CwshProcessMatchType match_type)
{
  CwshProcess *process = NULL;

  ProcessList::iterator p1 = processes_.begin();
  ProcessList::iterator p2 = processes_.end  ();

  for ( ; p1 != p2; ++p1) {
    CwshCommand *command = (*p1)->getCommand()->getCommand();

    CCommand::State state = command->getState();

    if (state != CCommand::RUNNING_STATE && state != CCommand::STOPPED_STATE)
      continue;

    string command_str = command->getCommandString();

    string::size_type pos = command_str.find(str);

    if ((match_type == CWSH_PROCESS_MATCH_START && pos == 0) ||
        (match_type == CWSH_PROCESS_MATCH_ANY   && pos != string::npos))
      process = *p1;
  }

  return process;
}

CwshProcess *
CwshProcessMgr::
lookupProcess(pid_t pid)
{
  ProcessList::iterator p1 = processes_.begin();
  ProcessList::iterator p2 = processes_.end  ();

  for ( ; p1 != p2; ++p1) {
    CwshProcess *process = *p1;

    if (process->isPid(pid))
      return process;
  }

  return NULL;
}

//--------

CwshProcess::
CwshProcess(CwshCommandData *command, int num) :
 command_(command), num_(num)
{
}

CwshProcess::
~CwshProcess()
{
}

void
CwshProcess::
addSubCommand(CwshCommandData *command)
{
  subCommands_.push_back(command);
}

CCommand::State
CwshProcess::
getCommandState() const
{
  return command_->getCommand()->getState();
}

string
CwshProcess::
getCommandString() const
{
  return command_->getCommand()->getCommandString();
}

pid_t
CwshProcess::
getCommandPid() const
{
  return command_->getCommand()->getPid();
}

void
CwshProcess::
start()
{
  command_->getCommand()->start();
}

void
CwshProcess::
stop()
{
  command_->getCommand()->stop();
}

void
CwshProcess::
tstop()
{
  command_->getCommand()->tstop();
}

void
CwshProcess::
pause()
{
  command_->getCommand()->pause();
}

void
CwshProcess::
resume()
{
  uint numSubCommands = subCommands_.size();

  for (uint i = 0; i < numSubCommands; ++i)
    subCommands_[i]->getCommand()->resume();

  command_->getCommand()->resume();
}

void
CwshProcess::
wait()
{
  command_->getCommand()->wait();
}

void
CwshProcess::
setNotify(bool flag)
{
  command_->getCommand()->setNotify(flag);
}

bool
CwshProcess::
isPid(pid_t pid) const
{
  pid_t pid1 = command_->getCommand()->getPid();

  if (pid1 == pid)
    return true;

  uint numSubCommands = subCommands_.size();

  for (uint i = 0; i < numSubCommands; ++i) {
    pid1 = subCommands_[i]->getCommand()->getPid();

    if (pid1 == pid)
      return true;
  }

  return false;
}

void
CwshProcess::
print() const
{
  std::cout << " " << command_->getCommand()->getCommandString();

  uint numSubCommands = subCommands_.size();

  for (uint i = 0; i < numSubCommands; ++i)
    std::cout << " | " << subCommands_[i]->getCommand()->getCommandString();
}
