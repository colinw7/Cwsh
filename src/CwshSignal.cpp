#include <CwshI.h>
#include <COSSignal.h>
#include <cerrno>

namespace Cwsh {

Signal
Signal::signals_[] = {
  Signal("HUP"   , SIGHUP   ),
  Signal("INT"   , SIGINT   ),
  Signal("QUIT"  , SIGQUIT  ),
  Signal("ILL"   , SIGILL   ),
  Signal("TRAP"  , SIGTRAP  ),
  Signal("ABRT"  , SIGABRT  ),
  Signal("IOT"   , SIGIOT   ),
  Signal("BUS"   , SIGBUS   ),
  Signal("FPE"   , SIGFPE   ),
  Signal("KILL"  , SIGKILL  ),
  Signal("USR1"  , SIGUSR1  ),
  Signal("SEGV"  , SIGSEGV  ),
  Signal("USR2"  , SIGUSR2  ),
  Signal("PIPE"  , SIGPIPE  ),
  Signal("ALRM"  , SIGALRM  ),
  Signal("TERM"  , SIGTERM  ),
#ifdef SIGSTKFLT
  Signal("STKFLT", SIGSTKFLT),
#endif
  Signal("CHLD"  , SIGCHLD  ),
  Signal("CONT"  , SIGCONT  ),
  Signal("STOP"  , SIGSTOP  ),
  Signal("TSTP"  , SIGTSTP  ),
  Signal("TTIN"  , SIGTTIN  ),
  Signal("TTOU"  , SIGTTOU  ),
  Signal("URG"   , SIGURG   ),
#ifdef SIGXCPU
  Signal("XCPU"  , SIGXCPU  ),
#endif
#ifdef SIGXFSZ
  Signal("XFSZ"  , SIGXFSZ  ),
#endif
#ifdef SIGVTALRM
  Signal("VTALRM", SIGVTALRM),
#endif
  Signal("PROF"  , SIGPROF  ),
  Signal("WINCH" , SIGWINCH ),
  Signal("IO"    , SIGIO    ),
#ifdef SIGPOLL
  Signal("POLL"  , SIGPOLL  ),
#endif
#ifdef SIGPWR
  Signal("PWR"   , SIGPWR   ),
#endif
#ifdef SIGSYS
  Signal("SYS"   , SIGSYS   ),
#endif
};

InterruptType Signal::interruptType_ = InterruptType::NORMAL;
std::string   Signal::interruptLabel_;

void
Signal::
addHandlers()
{
  COSSignal::addSignalHandler(SIGHUP  ,
    reinterpret_cast<COSSignal::SignalHandler>(termHandler     ));
  COSSignal::addSignalHandler(SIGINT  ,
    reinterpret_cast<COSSignal::SignalHandler>(interruptHandler));
  COSSignal::addSignalHandler(SIGQUIT ,
    reinterpret_cast<COSSignal::SignalHandler>(genericHandler  ));
  COSSignal::addSignalHandler(SIGILL  ,
    reinterpret_cast<COSSignal::SignalHandler>(genericHandler  ));
  COSSignal::addSignalHandler(SIGTRAP ,
    reinterpret_cast<COSSignal::SignalHandler>(genericHandler  ));
  COSSignal::addSignalHandler(SIGIOT  ,
    reinterpret_cast<COSSignal::SignalHandler>(genericHandler  ));
  COSSignal::addSignalHandler(SIGFPE  ,
    reinterpret_cast<COSSignal::SignalHandler>(genericHandler  ));
  COSSignal::addSignalHandler(SIGUSR1 ,
    reinterpret_cast<COSSignal::SignalHandler>(genericHandler  ));
  COSSignal::addSignalHandler(SIGUSR2 ,
    reinterpret_cast<COSSignal::SignalHandler>(genericHandler  ));
  COSSignal::addSignalHandler(SIGPIPE ,
    reinterpret_cast<COSSignal::SignalHandler>(genericHandler  ));
  COSSignal::addSignalHandler(SIGALRM ,
    reinterpret_cast<COSSignal::SignalHandler>(genericHandler  ));
  COSSignal::addSignalHandler(SIGTERM ,
    reinterpret_cast<COSSignal::SignalHandler>(genericHandler  ));
  COSSignal::addSignalHandler(SIGCONT ,
    reinterpret_cast<COSSignal::SignalHandler>(genericHandler  ));
  COSSignal::addSignalHandler(SIGTSTP ,
    reinterpret_cast<COSSignal::SignalHandler>(stopHandler     ));
  COSSignal::addSignalHandler(SIGTTIN ,
    reinterpret_cast<COSSignal::SignalHandler>(SIG_IGN         ));
  COSSignal::addSignalHandler(SIGTTOU ,
    reinterpret_cast<COSSignal::SignalHandler>(genericHandler  ));
  COSSignal::addSignalHandler(SIGWINCH,
    reinterpret_cast<COSSignal::SignalHandler>(genericHandler  ));
}

void
Signal::
nohup()
{
  COSSignal::addSignalHandler(SIGHUP, (COSSignal::SignalHandler) SIG_IGN);
}

void
Signal::
termHandler(int)
{
  auto *mgr = CwshMgrInst;

  mgr->term(0);
}

// TODO: use static of type 'volatile sig_atomic_t flag'

// TODO: use sigsetjmp, siglongjmp to handle interrupt and restart readline loop
// and ensure signal handling is not changed
void
Signal::
interruptHandler(int)
{
  auto *mgr = CwshMgrInst;

  if      (interruptType_ == InterruptType::NORMAL) {
    mgr->setInterrupt(true);

    mgr->readInterrupt();
  }
  else if (interruptType_ == InterruptType::GOTO)
    mgr->gotoBlockLabel(interruptLabel_);
}

void
Signal::
stopHandler(int)
{
  int savedErrno = errno; // In case we change 'errno'

#if 0
  //printf("Caught SIGTSTP\n"); /* UNSAFE (see Section 21.1.2) */

  if (signal(SIGTSTP, SIG_DFL) == SIG_ERR)
    errExit("signal"); /* Set handling to default */

  raise(SIGTSTP); /* Generate a further SIGTSTP */

  /* Unblock SIGTSTP; the pending SIGTSTP immediately suspends the program */

  COSSignal::unblockSignal(SIGTSTP);

  sigset_t prevMask = COSSignal::getOldSigSet();

  /* Execution resumes here after SIGCONT */

  COSSignal::setSigProcMask(prevMask);

  COSSignal::addSignalHandler(SIGTSTP, stopHandler, CSIGNAL_OPT_RESTART);

  //printf("Exiting SIGTSTP handler\n");
#else
  std::cerr << "stopHandler\n";

  auto *mgr = CwshMgrInst;

  mgr->stopActiveProcesses();
#endif

  errno = savedErrno;
}

void
Signal::
genericHandler(int)
{
#if 0
  auto *sig = lookup(num);

  std::cerr << "Signal " << sig->name_ << "(" << sig->num_ << ") received\n";
#endif
}

Signal::
Signal(const std::string &name, int num) :
 name_(name), num_(num)
{
}

Signal *
Signal::
lookup(const std::string &name)
{
  int num_signals = getNumSignals();

  for (int i = 0; i < num_signals; i++)
    if (name == signals_[i].name_)
      return &signals_[i];

  return nullptr;
}

Signal *
Signal::
lookup(int num)
{
  int num_signals = getNumSignals();

  for (int i = 0; i < num_signals; i++)
    if (num == signals_[i].num_)
      return &signals_[i];

  return nullptr;
}

int
Signal::
getNumSignals()
{
  return sizeof(signals_)/sizeof(Signal);
}

Signal *
Signal::
getSignal(int i)
{
  return &signals_[i];
}

}
