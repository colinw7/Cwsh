#include <CwshI.h>
#include <CwshHistoryParser.h>

CwshHistory::
CwshHistory(Cwsh *cwsh) :
 cwsh_(cwsh)
{
  std::string filename = getPath();

  addFile(filename);
}

CwshHistory::
~CwshHistory()
{
  updateSize();

  int save_size = getSaveSize();

  if (save_size < getSize())
    history_.resize(save_size);

  if (save_size > 0) {
    std::string filename = getPath();

    history_.save(filename);
  }
}

bool
CwshHistory::
findCommandStart(const std::string &str, int &command_num)
{
  std::string command;

  if (history1_.findCommandStart(str, command, command_num)) {
    command_num = history_.getCommandNum() + 1;
    return true;
  }

  updateSize();

  return history_.findCommandStart(str, command, command_num);
}

bool
CwshHistory::
findCommandIn(const std::string &str, int &command_num)
{
  std::string command;

  if (history1_.findCommandIn(str, command, command_num)) {
    command_num = history_.getCommandNum() + 1;
    return true;
  }

  updateSize();

  return history_.findCommandIn(str, command, command_num);
}

bool
CwshHistory::
findCommandArg(const std::string &str, int &command_num, int &arg_num)
{
  if (history1_.findCommandArg(str, command_num, arg_num)) {
    command_num = history_.getCommandNum() + 1;
    return true;
  }

  updateSize();

  return history_.findCommandArg(str, command_num, arg_num);
}

std::string
CwshHistory::
getCommand(int num)
{
  updateSize();

  std::string command;

  if (! history_.getCommand(num, command))
    CWSH_THROW("Invalid history command num");

  return command;
}

std::string
CwshHistory::
getCommandArg(int num, int arg_num)
{
  updateSize();

  std::string command, arg;

  if (! history_.getCommandArg(num, arg_num, command, arg))
    CWSH_THROW("Invalid history command/arg num");

  return arg;
}

void
CwshHistory::
addFile(const std::string &filename)
{
  history_.addFile(filename);

  updateSize();

  command_num_ = history_.getLastCommandNum() + 1;
}

void
CwshHistory::
addCommand(const std::string &line)
{
  std::string line1 = CStrUtil::stripSpaces(line);

  if (line1 == "")
    return;

  setCurrent(line1);

  history_.addCommand(line1);

  updateSize();

  command_num_ = history_.getLastCommandNum() + 1;
}

void
CwshHistory::
setCurrent(const std::string &line)
{
  std::string line1 = CStrUtil::stripSpaces(line);

  if (line1 == "")
    return;

  history1_.addCommand(line1);

  history1_.resize(1);
}

void
CwshHistory::
display(int num, bool show_numbers, bool show_time, bool reverse)
{
  updateSize();

  if (num > getSize())
    num = -1;

  history_.display(num, show_numbers, show_time, reverse);
}

void
CwshHistory::
updateSize()
{
  int size = getSize();

  history_.resize(size);
}

int
CwshHistory::
getSize() const
{
  auto *variable = cwsh_->lookupVariable("history");

  if (! variable)
    return 1;

  if (variable->getNumValues() != 1)
    return 1;

  std::string value = variable->getValue(0);

  if (! CStrUtil::isInteger(value))
    return 1;

  int size = int(CStrUtil::toInteger(variable->getValue(0)));

  if (size < 1)
    return 1;

  return size;
}

int
CwshHistory::
getSaveSize() const
{
  auto *variable = cwsh_->lookupVariable("savehist");

  if (! variable)
    return 0;

  if (variable->getNumValues() != 1)
    CWSH_THROW("Badly formed number.");

  std::string value = variable->getValue(0);

  if (! CStrUtil::isInteger(value))
    CWSH_THROW("Badly formed number.");

  return int(CStrUtil::toInteger(value));
}

bool
CwshHistory::
hasPrevCommand()
{
  std::string command;

  if (command_num_ <= 0 || ! history_.getCommand(command_num_ - 1, command))
    return false;

  return true;
}

bool
CwshHistory::
hasNextCommand()
{
  return true;
}

std::string
CwshHistory::
getPrevCommand()
{
  std::string command;

  if      (  history_.getCommand(command_num_ - 1, command))
    --command_num_;
  else if (! history_.getCommand(command_num_, command))
    command = "";

  return command;
}

std::string
CwshHistory::
getNextCommand()
{
  std::string command;

  if (! history_.getCommand(command_num_ + 1, command))
    return "";

  ++command_num_;

  return command;
}

std::string
CwshHistory::
getPath()
{
  return CStrUtil::concatFileNames(COSUser::getUserHome(), getFilename());
}

std::string
CwshHistory::
getFilename()
{
  std::string filename = ".history";

  return filename;
}

std::string
CwshHistory::
getHistoryMsg() const
{
  std::string msg;

  CHistory::CommandList::const_iterator p1, p2;

  for (p1 = history_.beginCommand(), p2 = history_.endCommand(); p1 != p2; ++p1) {
    if (! msg.empty()) msg += "#";

    msg += CStrUtil::toString((*p1)->getNumber()) + "#";
    msg += (*p1)->getTimeString() + "#";
    msg += (*p1)->getCommand();
  }

  return msg;
}
