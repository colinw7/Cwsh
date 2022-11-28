#include <CwshI.h>
#include <CwshHistoryParser.h>

namespace Cwsh {

History::
History(App *cwsh) :
 cwsh_(cwsh)
{
  auto filename = getPath();

  addFile(filename);
}

History::
~History()
{
  updateSize();

  int saveSize = getSaveSize();

  if (saveSize < getSize())
    history_.resize(saveSize);

  if (saveSize > 0) {
    auto filename = getPath();

    history_.save(filename);
  }
}

bool
History::
findCommandStart(const std::string &str, int &commandNum)
{
  std::string command;

  if (history1_.findCommandStart(str, command, commandNum)) {
    commandNum = history_.getCommandNum() + 1;
    return true;
  }

  updateSize();

  return history_.findCommandStart(str, command, commandNum);
}

bool
History::
findCommandIn(const std::string &str, int &commandNum)
{
  std::string command;

  if (history1_.findCommandIn(str, command, commandNum)) {
    commandNum = history_.getCommandNum() + 1;
    return true;
  }

  updateSize();

  return history_.findCommandIn(str, command, commandNum);
}

bool
History::
findCommandArg(const std::string &str, int &commandNum, int &argNum)
{
  if (history1_.findCommandArg(str, commandNum, argNum)) {
    commandNum = history_.getCommandNum() + 1;
    return true;
  }

  updateSize();

  return history_.findCommandArg(str, commandNum, argNum);
}

std::string
History::
getCommand(int num)
{
  updateSize();

  std::string command;

  if (! history_.getCommand(num, command))
    CWSH_THROW("Invalid history command num");

  return command;
}

std::string
History::
getCommandArg(int num, int argNum)
{
  updateSize();

  std::string command, arg;

  if (! history_.getCommandArg(num, argNum, command, arg))
    CWSH_THROW("Invalid history command/arg num");

  return arg;
}

void
History::
addFile(const std::string &filename)
{
  history_.addFile(filename);

  updateSize();

  commandNum_ = history_.getLastCommandNum() + 1;
}

void
History::
addCommand(const std::string &line)
{
  auto line1 = CStrUtil::stripSpaces(line);
  if (line1 == "") return;

  setCurrent(line1);

  history_.addCommand(line1);

  updateSize();

  commandNum_ = history_.getLastCommandNum() + 1;
}

void
History::
setCurrent(const std::string &line)
{
  auto line1 = CStrUtil::stripSpaces(line);
  if (line1 == "") return;

  history1_.addCommand(line1);

  history1_.resize(1);
}

void
History::
display(int num, bool showNumbers, bool showTime, bool reverse) const
{
  updateSize();

  if (num > getSize())
    num = -1;

  history_.display(num, showNumbers, showTime, reverse);
}

void
History::
updateSize() const
{
  int size = getSize();

  auto *th = const_cast<History *>(this);

  th->history_.resize(size);
}

int
History::
getSize() const
{
  auto *variable = cwsh_->lookupVariable("history");

  if (! variable)
    return 1;

  if (variable->getNumValues() != 1)
    return 1;

  auto value = variable->getValue(0);

  if (! CStrUtil::isInteger(value))
    return 1;

  int size = int(CStrUtil::toInteger(variable->getValue(0)));

  if (size < 1)
    return 1;

  return size;
}

int
History::
getSaveSize() const
{
  auto *variable = cwsh_->lookupVariable("savehist");

  if (! variable)
    return 0;

  if (variable->getNumValues() != 1)
    CWSH_THROW("Badly formed number.");

  auto value = variable->getValue(0);

  if (! CStrUtil::isInteger(value))
    CWSH_THROW("Badly formed number.");

  return int(CStrUtil::toInteger(value));
}

bool
History::
hasPrevCommand()
{
  std::string command;

  if (commandNum_ <= 0 || ! history_.getCommand(commandNum_ - 1, command))
    return false;

  return true;
}

bool
History::
hasNextCommand()
{
  return true;
}

std::string
History::
getPrevCommand()
{
  std::string command;

  if      (  history_.getCommand(commandNum_ - 1, command))
    --commandNum_;
  else if (! history_.getCommand(commandNum_, command))
    command = "";

  return command;
}

std::string
History::
getNextCommand()
{
  std::string command;

  if (! history_.getCommand(commandNum_ + 1, command))
    return "";

  ++commandNum_;

  return command;
}

std::string
History::
getPath()
{
  return CStrUtil::concatFileNames(COSUser::getUserHome(), getFilename());
}

std::string
History::
getFilename()
{
  auto filename = std::string(".history");

  return filename;
}

std::string
History::
getHistoryMsg() const
{
  std::string msg;

  for (auto p1 = history_.beginCommand(); p1 != history_.endCommand(); ++p1) {
    if (! msg.empty()) msg += "#";

    msg += CStrUtil::toString((*p1)->getNumber()) + "#";
    msg += (*p1)->getTimeString() + "#";
    msg += (*p1)->getCommand();
  }

  return msg;
}

}
