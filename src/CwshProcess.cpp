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

  auto *process = new CwshProcess(command, num);

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

  if (! process) {
    int error = COSProcess::killProcess(pid, signal);

    if (error < 0)
      CWSH_THROW("No such process");

    return;
  }

  CCommand::State state = process->getCommandState();

  if (state == CCommand::State::RUNNING || state == CCommand::State::STOPPED) {
    int error_code = COSProcess::killProcess(pid, signal);

    if (error_code != 0)
      CWSH_THROW(std::string("kill: ") + strerror(errno) + ".");
  }
}

int
CwshProcessMgr::
getNumActive()
{
  int count  = 0;

  for (auto &process : processes_) {
    CCommand::State state = process->getCommandState();

    if (state != CCommand::State::RUNNING && state != CCommand::State::STOPPED)
      continue;

    count++;
  }

  return count;
}

void
CwshProcessMgr::
displayActive(bool list_pids)
{
  std::vector<CwshProcess *> active_processes;

  for (auto &process : processes_) {
    CCommand::State state = process->getCommandState();

    if (state == CCommand::State::EXITED)
      continue;

    if (! process->getCommand()->getCommand()->getDoFork())
      continue;

    active_processes.push_back(process);
  }

  uint num_active_processes = uint(active_processes.size());

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

    if      (process->getCommandState() == CCommand::State::STOPPED)
      std::cout << "Suspended             ";
    else
      std::cout << "Running               ";

    process->print();

    std::cout << "\n";
  }

  deleteExited();
}

void
CwshProcessMgr::
displayExited()
{
  int count = 0;

  ProcessList::iterator p1 = processes_.begin();

  while (p1 != processes_.end()) {
    CwshProcess *process = *p1;

    CCommand::State state = process->getCommandState();

    if (state == CCommand::State::RUNNING || state == CCommand::State::STOPPED ||
        state == CCommand::State::EXITED)
      count++;

    if (state == CCommand::State::EXITED) {
      std::cout << "[" << process->getNum() << "]    Done                  ";

      process->print();

      std::cout << "\n";

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
  for (auto &process : processes_) {
    CCommand::State state = process->getCommandState();

    if (state == CCommand::State::EXITED)
      remove(process);
  }
}

void
CwshProcessMgr::
waitActive()
{
  for (auto &process : processes_) {
    CwshCommand *command = process->getCommand()->getCommand();

    if (! command->getDoFork())
      continue;

    command->wait();
  }

  displayExited();
}

pid_t
CwshProcessMgr::
stringToPid(const std::string &str)
{
  CwshProcess *process = getActiveProcess(str);

  return process->getCommandPid();
}

CwshProcess *
CwshProcessMgr::
getActiveProcess(const std::string &str)
{
  if (str.size() < 1 || str[0] != '%')
    CWSH_THROW("No current job.");

  CwshProcess *process = nullptr;

  if      (str.size() == 1 ||
           (str.size() == 2 && (str[1] == '%' || str[1] == '+')))
    process = getCurrentActiveProcess();
  else if (str.size() == 2 && str[1] == '-')
    process = getPreviousActiveProcess();
  else if (str.size() > 1 && CStrUtil::isInteger(str.substr(1))) {
    int process_num = int(CStrUtil::toInteger(str.substr(1)));

    process = getActiveProcess(process_num);
  }
  else if (str[1] == '?')
    process = matchActiveProcess(str.substr(1), CwshProcessMatchType::ANY);
  else
    process = matchActiveProcess(str.substr(1), CwshProcessMatchType::START);

  if (! process)
    CWSH_THROW("No such job.");

  return process;
}

CwshProcess *
CwshProcessMgr::
getCurrentActiveProcess()
{
  CwshProcess *currentProcess = nullptr;

  for (auto &process : processes_) {
    CwshCommand *command = process->getCommand()->getCommand();

    CCommand::State state = command->getState();

    if (state != CCommand::State::RUNNING && state != CCommand::State::STOPPED)
      continue;

    currentProcess = process;
  }

  return currentProcess;
}

CwshProcess *
CwshProcessMgr::
getPreviousActiveProcess()
{
  CwshProcess *process1 = nullptr;
  CwshProcess *process2 = nullptr;

  for (auto &process : processes_) {
    CwshCommand *command = process->getCommand()->getCommand();

    CCommand::State state = command->getState();

    if (state != CCommand::State::RUNNING && state != CCommand::State::STOPPED)
      continue;

    process1 = process2;
    process2 = process;
  }

  return process1;
}

CwshProcess *
CwshProcessMgr::
getActiveProcess(int num)
{
  std::vector<CwshProcess *> active_processes;

  for (auto &process : processes_) {
    CwshCommand *command = process->getCommand()->getCommand();

    CCommand::State state = command->getState();

    if (state != CCommand::State::RUNNING && state != CCommand::State::STOPPED)
      continue;

    active_processes.push_back(process);
  }

  int num_active_processes = int(active_processes.size());

  if (num > num_active_processes)
    return nullptr;

  return active_processes[num_active_processes - num];
}

CwshProcess *
CwshProcessMgr::
matchActiveProcess(const std::string &str, CwshProcessMatchType match_type)
{
  CwshProcess *activeProcess = nullptr;

  for (auto &process : processes_) {
    CwshCommand *command = process->getCommand()->getCommand();

    CCommand::State state = command->getState();

    if (state != CCommand::State::RUNNING && state != CCommand::State::STOPPED)
      continue;

    std::string command_str = command->getCommandString();

    std::string::size_type pos = command_str.find(str);

    if ((match_type == CwshProcessMatchType::START && pos == 0) ||
        (match_type == CwshProcessMatchType::ANY   && pos != std::string::npos))
      activeProcess = process;
  }

  return activeProcess;
}

CwshProcess *
CwshProcessMgr::
lookupProcess(pid_t pid)
{
  for (auto &process : processes_) {
    if (process->isPid(pid))
      return process;
  }

  return nullptr;
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

std::string
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
  uint numSubCommands = uint(subCommands_.size());

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

  uint numSubCommands = uint(subCommands_.size());

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

  uint numSubCommands = uint(subCommands_.size());

  for (uint i = 0; i < numSubCommands; ++i)
    std::cout << " | " << subCommands_[i]->getCommand()->getCommandString();
}
