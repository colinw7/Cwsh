#include <CwshI.h>
#include <CReadLine.h>
#include <cstdio>

CwshReadLine::
CwshReadLine(Cwsh *cwsh) :
 cwsh_(cwsh)
{
}

std::string
CwshReadLine::
readLine()
{
  std::string prompt = cwsh_->getInputPrompt();

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
  catch (struct CwshErr *cthrow) {
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
CwshReadLine::
completeLine(const std::string &line, std::string &line1)
{
  CwshComplete complete(cwsh_, line);

  if (! complete.complete(line1))
    cwsh_->beep();

  return true;
}

bool
CwshReadLine::
showComplete(const std::string &line)
{
  CwshMatch match(cwsh_);

  return match.showMatch(line);
}

bool
CwshReadLine::
getPrevCommand(std::string &line)
{
  if (! cwsh_->hasPrevHistoryCommand())
    return false;

  line = cwsh_->getPrevHistoryCommand();

  return true;
}

bool
CwshReadLine::
getNextCommand(std::string &line)
{
  if (! cwsh_->hasNextHistoryCommand())
    return false;

  line = cwsh_->getNextHistoryCommand();

  return true;
}

void
CwshReadLine::
beep()
{
  auto *nobeep = cwsh_->lookupVariable("nobeep");

  if (! nobeep)
    CReadLine::beep();
}

void
CwshReadLine::
interrupt()
{
  CReadLine::interrupt();
}

void
CwshReadLine::
timeout()
{
  cwsh_->readTimeout();
}
