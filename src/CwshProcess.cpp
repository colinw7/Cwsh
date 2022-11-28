#include <CwshI.h>
#include <COSProcess.h>
#include <cstring>
#include <cerrno>

namespace Cwsh {

ProcessMgr::
ProcessMgr(App *cwsh) :
 cwsh_(cwsh)
{
}

Process *
ProcessMgr::
add(CommandData *command)
{
  int num = 1;

  if (processes_.size() > 0)
    num = processes_.back()->getNum() + 1;

  auto process = std::make_shared<Process>(command, num);

  processes_.push_back(process);

  return process.get();
}

void
ProcessMgr::
remove(Process *process)
{
  for (const auto &process1 : processes_) {
    if (process == process1.get()) {
      remove(process1);
      break;
    }
  }
}

void
ProcessMgr::
remove(ProcessP process)
{
  processes_.remove(process);
}

void
ProcessMgr::
kill(pid_t pid, int signal)
{
  auto *process = lookupProcess(pid);

  if (! process) {
    int error = COSProcess::killProcess(pid, signal);

    if (error < 0)
      CWSH_THROW("No such process");

    return;
  }

  CCommand::State state = process->getCommandState();

  if (state == CCommand::State::RUNNING || state == CCommand::State::STOPPED) {
    int errorCode = COSProcess::killProcess(pid, signal);

    if (errorCode != 0)
      CWSH_THROW(std::string("kill: ") + strerror(errno) + ".");
  }
}

int
ProcessMgr::
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
ProcessMgr::
displayActive(bool listPids)
{
  std::vector<ProcessP> activeProcesses;

  for (auto &process : processes_) {
    CCommand::State state = process->getCommandState();

    if (state == CCommand::State::EXITED)
      continue;

    if (! process->getCommand()->getCommand()->getDoFork())
      continue;

    activeProcesses.push_back(process);
  }

  uint numActiveProcesses = uint(activeProcesses.size());

  for (uint i = 0; i < numActiveProcesses; i++) {
    auto process = activeProcesses[i];

    std::cout << "[" << process->getNum() << "]  ";

    if      (i == numActiveProcesses - 1)
      std::cout << "+ ";
    else if (i == numActiveProcesses - 2)
      std::cout << "- ";
    else
      std::cout << "  ";

    if (listPids)
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
ProcessMgr::
displayExited()
{
  int count = 0;

  auto p1 = processes_.begin();

  while (p1 != processes_.end()) {
    auto process = *p1;

    auto state = process->getCommandState();

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
ProcessMgr::
deleteExited()
{
  for (auto &process : processes_) {
    auto state = process->getCommandState();

    if (state == CCommand::State::EXITED)
      remove(process);
  }
}

void
ProcessMgr::
waitActive()
{
  for (auto &process : processes_) {
    auto *command = process->getCommand()->getCommand();

    if (! command->getDoFork())
      continue;

    command->wait();
  }

  displayExited();
}

pid_t
ProcessMgr::
stringToPid(const std::string &str)
{
  auto *process = getActiveProcess(str);

  return process->getCommandPid();
}

Process *
ProcessMgr::
getActiveProcess(const std::string &str)
{
  if (str.size() < 1 || str[0] != '%')
    CWSH_THROW("No current job.");

  Process *process = nullptr;

  if      (str.size() == 1 ||
           (str.size() == 2 && (str[1] == '%' || str[1] == '+')))
    process = getCurrentActiveProcess();
  else if (str.size() == 2 && str[1] == '-')
    process = getPreviousActiveProcess();
  else if (str.size() > 1 && CStrUtil::isInteger(str.substr(1))) {
    int processNum = int(CStrUtil::toInteger(str.substr(1)));

    process = getActiveProcess(processNum);
  }
  else if (str[1] == '?')
    process = matchActiveProcess(str.substr(1), ProcessMatchType::ANY);
  else
    process = matchActiveProcess(str.substr(1), ProcessMatchType::START);

  if (! process)
    CWSH_THROW("No such job.");

  return process;
}

Process *
ProcessMgr::
getCurrentActiveProcess()
{
  ProcessP currentProcess;

  for (auto &process : processes_) {
    auto *command = process->getCommand()->getCommand();

    CCommand::State state = command->getState();

    if (state != CCommand::State::RUNNING && state != CCommand::State::STOPPED)
      continue;

    currentProcess = process;
  }

  return currentProcess.get();
}

Process *
ProcessMgr::
getPreviousActiveProcess()
{
  ProcessP process1, process2;

  for (auto &process : processes_) {
    auto *command = process->getCommand()->getCommand();

    CCommand::State state = command->getState();

    if (state != CCommand::State::RUNNING && state != CCommand::State::STOPPED)
      continue;

    process1 = process2;
    process2 = process;
  }

  return process1.get();
}

Process *
ProcessMgr::
getActiveProcess(int num)
{
  std::vector<ProcessP> activeProcesses;

  for (auto &process : processes_) {
    auto *command = process->getCommand()->getCommand();

    CCommand::State state = command->getState();

    if (state != CCommand::State::RUNNING && state != CCommand::State::STOPPED)
      continue;

    activeProcesses.push_back(process);
  }

  int numActiveProcesses = int(activeProcesses.size());

  if (num > numActiveProcesses)
    return nullptr;

  return activeProcesses[numActiveProcesses - num].get();
}

Process *
ProcessMgr::
matchActiveProcess(const std::string &str, ProcessMatchType matchType)
{
  ProcessP activeProcess;

  for (auto &process : processes_) {
    auto *command = process->getCommand()->getCommand();

    CCommand::State state = command->getState();

    if (state != CCommand::State::RUNNING && state != CCommand::State::STOPPED)
      continue;

    std::string commandStr = command->getCommandString();

    std::string::size_type pos = commandStr.find(str);

    if ((matchType == ProcessMatchType::START && pos == 0) ||
        (matchType == ProcessMatchType::ANY   && pos != std::string::npos))
      activeProcess = process;
  }

  return activeProcess.get();
}

Process *
ProcessMgr::
lookupProcess(pid_t pid)
{
  for (auto &process : processes_) {
    if (process->isPid(pid))
      return process.get();
  }

  return nullptr;
}

//--------

Process::
Process(CommandData *command, int num) :
 command_(command), num_(num)
{
}

Process::
~Process()
{
}

void
Process::
addSubCommand(CommandData *command)
{
  subCommands_.push_back(command);
}

CCommand::State
Process::
getCommandState() const
{
  return command_->getCommand()->getState();
}

std::string
Process::
getCommandString() const
{
  return command_->getCommand()->getCommandString();
}

pid_t
Process::
getCommandPid() const
{
  return command_->getCommand()->getPid();
}

void
Process::
start()
{
  command_->getCommand()->start();
}

void
Process::
stop()
{
  command_->getCommand()->stop();
}

void
Process::
tstop()
{
  command_->getCommand()->tstop();
}

void
Process::
pause()
{
  command_->getCommand()->pause();
}

void
Process::
resume()
{
  auto numSubCommands = uint(subCommands_.size());

  for (uint i = 0; i < numSubCommands; ++i)
    subCommands_[i]->getCommand()->resume();

  command_->getCommand()->resume();
}

void
Process::
wait()
{
  command_->getCommand()->wait();
}

void
Process::
setNotify(bool flag)
{
  command_->getCommand()->setNotify(flag);
}

bool
Process::
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
Process::
print() const
{
  std::cout << " " << command_->getCommand()->getCommandString();

  uint numSubCommands = uint(subCommands_.size());

  for (uint i = 0; i < numSubCommands; ++i)
    std::cout << " | " << subCommands_[i]->getCommand()->getCommandString();
}

}
