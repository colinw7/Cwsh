#include "CwshI.h"
#include <CReadLine.h>
#include <cstdio>

CwshReadLine::
CwshReadLine(Cwsh *cwsh) :
 cwsh_(cwsh)
{
}

string
CwshReadLine::
readLine()
{
  string prompt = cwsh_->getInputPrompt();

  setPrompt(prompt);

  fflush(stdout);
  fflush(stderr);

  string line;

  try {
    line = CReadLine::readLine();

    if (eof()) {
      cout << endl;

      CwshVariable *variable = cwsh_->lookupVariable("ignoreeof");

      if (variable == NULL)
        cwsh_->setExit(true, 0);
      else
        CWSH_THROW("Use \"exit\" to leave shell.");
    }
  }
  catch (struct CwshErr *cthrow) {
    if (cwsh_->getDebug())
      cerr << "[" << cthrow->file << ":" << cthrow->line << "] ";

    if (cthrow->qualifier != "")
      cerr << cthrow->qualifier << ": " << cthrow->message << endl;
    else
      cerr << cthrow->message << endl;
  }
  catch (...) {
    cerr << "Unhandled Exception thrown" << endl;
  }

  return line;
}

bool
CwshReadLine::
completeLine(const string &line, string &line1)
{
  CwshComplete complete(cwsh_, line);

  if (! complete.complete(line1))
    cwsh_->beep();

  return true;
}

bool
CwshReadLine::
showComplete(const string &line)
{
  CwshMatch match(cwsh_);

  return match.showMatch(line);
}

bool
CwshReadLine::
getPrevCommand(string &line)
{
  if (! cwsh_->hasPrevHistoryCommand())
    return false;

  line = cwsh_->getPrevHistoryCommand();

  return true;
}

bool
CwshReadLine::
getNextCommand(string &line)
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
  CwshVariable *nobeep = cwsh_->lookupVariable("nobeep");

  if (nobeep == NULL)
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
