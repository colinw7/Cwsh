#include <CwshI.h>
#include <CReadLine.h>
#include <cstdio>

namespace Cwsh {

ReadLine::
ReadLine(App *cwsh) :
 cwsh_(cwsh)
{
}

std::string
ReadLine::
readLine()
{
  auto prompt = cwsh_->getInputPrompt();

  setPrompt(prompt);

  fflush(stdout);
  fflush(stderr);

  std::string line;

  try {
    line = CReadLine::readLine();

    if (eof()) {
      std::cout << "\n";

      auto *variable = cwsh_->lookupVariable("ignoreeof");

      if (! variable)
        cwsh_->setExit(true, 0);
      else
        CWSH_THROW("Use \"exit\" to leave shell.");
    }
  }
  catch (struct Err *cthrow) {
    if (cwsh_->getDebug())
      std::cerr << "[" << cthrow->file << ":" << cthrow->line << "] ";

    if (cthrow->qualifier != "")
      std::cerr << cthrow->qualifier << ": " << cthrow->message << "\n";
    else
      std::cerr << cthrow->message << "\n";
  }
  catch (...) {
    std::cerr << "Unhandled Exception thrown\n";
  }

  return line;
}

bool
ReadLine::
completeLine(const std::string &line, std::string &line1)
{
  Complete complete(cwsh_, line);

  if (! complete.complete(line1))
    cwsh_->beep();

  return true;
}

bool
ReadLine::
showComplete(const std::string &line)
{
  Match match(cwsh_);

  return match.showMatch(line);
}

bool
ReadLine::
getPrevCommand(std::string &line)
{
  if (! cwsh_->hasPrevHistoryCommand())
    return false;

  line = cwsh_->getPrevHistoryCommand();

  return true;
}

bool
ReadLine::
getNextCommand(std::string &line)
{
  if (! cwsh_->hasNextHistoryCommand())
    return false;

  line = cwsh_->getNextHistoryCommand();

  return true;
}

void
ReadLine::
beep()
{
  auto *nobeep = cwsh_->lookupVariable("nobeep");

  if (! nobeep)
    CReadLine::beep();
}

void
ReadLine::
interrupt()
{
  CReadLine::interrupt();
}

void
ReadLine::
timeout()
{
  cwsh_->readTimeout();
}

}
