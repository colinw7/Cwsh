#include <CwshI.h>
#include <COSSignal.h>
#include <cerrno>

CwshSignal
CwshSignal::signals_[] = {
  CwshSignal("HUP"   , SIGHUP   ),
  CwshSignal("INT"   , SIGINT   ),
  CwshSignal("QUIT"  , SIGQUIT  ),
  CwshSignal("ILL"   , SIGILL   ),
  CwshSignal("TRAP"  , SIGTRAP  ),
  CwshSignal("ABRT"  , SIGABRT  ),
  CwshSignal("IOT"   , SIGIOT   ),
  CwshSignal("BUS"   , SIGBUS   ),
  CwshSignal("FPE"   , SIGFPE   ),
  CwshSignal("KILL"  , SIGKILL  ),
  CwshSignal("USR1"  , SIGUSR1  ),
  CwshSignal("SEGV"  , SIGSEGV  ),
  CwshSignal("USR2"  , SIGUSR2  ),
  CwshSignal("PIPE"  , SIGPIPE  ),
  CwshSignal("ALRM"  , SIGALRM  ),
  CwshSignal("TERM"  , SIGTERM  ),
#ifdef SIGSTKFLT
  CwshSignal("STKFLT", SIGSTKFLT),
#endif
  CwshSignal("CHLD"  , SIGCHLD  ),
  CwshSignal("CONT"  , SIGCONT  ),
  CwshSignal("STOP"  , SIGSTOP  ),
  CwshSignal("TSTP"  , SIGTSTP  ),
  CwshSignal("TTIN"  , SIGTTIN  ),
  CwshSignal("TTOU"  , SIGTTOU  ),
  CwshSignal("URG"   , SIGURG   ),
#ifdef SIGXCPU
  CwshSignal("XCPU"  , SIGXCPU  ),
#endif
#ifdef SIGXFSZ
  CwshSignal("XFSZ"  , SIGXFSZ  ),
#endif
#ifdef SIGVTALRM
  CwshSignal("VTALRM", SIGVTALRM),
#endif
  CwshSignal("PROF"  , SIGPROF  ),
  CwshSignal("WINCH" , SIGWINCH ),
  CwshSignal("IO"    , SIGIO    ),
#ifdef SIGPOLL
  CwshSignal("POLL"  , SIGPOLL  ),
#endif
#ifdef SIGPWR
  CwshSignal("PWR"   , SIGPWR   ),
#endif
#ifdef SIGSYS
  CwshSignal("SYS"   , SIGSYS   ),
#endif
};

CwshInterruptType CwshSignal::interrupt_type_ = CwshInterruptType::NORMAL;
std::string       CwshSignal::interrupt_label_;

void
CwshSignal::
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
CwshSignal::
nohup()
{
  COSSignal::addSignalHandler(SIGHUP, (COSSignal::SignalHandler) SIG_IGN);
}

void
CwshSignal::
termHandler(int)
{
  CwshMgrInst.term(0);
}

// TODO: use static of type 'volatile sig_atomic_t flag'

// TODO: use sigsetjmp, siglongjmp to handle interrupt and restart readline loop
// and ensure signal handling is not changed
void
CwshSignal::
interruptHandler(int)
{
  if      (interrupt_type_ == CwshInterruptType::NORMAL) {
    CwshMgrInst.setInterrupt(true);

    CwshMgrInst.readInterrupt();
  }
  else if (interrupt_type_ == CwshInterruptType::GOTO)
    CwshMgrInst.gotoBlockLabel(interrupt_label_);
}

void
CwshSignal::
stopHandler(int)
{
  int savedErrno = errno; // In case we change 'errno'

#if 0
  //printf("Caught SIGTSTP\n");         /* UNSAFE (see Section 21.1.2) */

  if (signal(SIGTSTP, SIG_DFL) == SIG_ERR)
    errExit("signal");              /* Set handling to default */

  raise(SIGTSTP);                     /* Generate a further SIGTSTP */

  /* Unblock SIGTSTP; the pending SIGTSTP immediately suspends the program */

  COSSignal::unblockSignal(SIGTSTP);

  sigset_t prevMask = COSSignal::getOldSigSet();

  /* Execution resumes here after SIGCONT */

  COSSignal::setSigProcMask(prevMask);

  COSSignal::addSignalHandler(SIGTSTP, stopHandler, CSIGNAL_OPT_RESTART);

  //printf("Exiting SIGTSTP handler\n");
#else
  std::cerr << "stopHandler" << std::endl;

  CwshMgrInst.stopActiveProcesses();
#endif

  errno = savedErrno;
}

void
CwshSignal::
genericHandler(int)
{
#if 0
  CwshSignal *sig = lookup(num);

  std::cerr << "Signal " << sig->name_ << "(" << sig->num_ << ") received" << std::endl;
#endif
}

CwshSignal::
CwshSignal(const std::string &name, int num) :
 name_(name), num_(num)
{
}

CwshSignal *
CwshSignal::
lookup(const std::string &name)
{
  int num_signals = getNumSignals();

  for (int i = 0; i < num_signals; i++)
    if (name == signals_[i].name_)
      return &signals_[i];

  return nullptr;
}

CwshSignal *
CwshSignal::
lookup(int num)
{
  int num_signals = getNumSignals();

  for (int i = 0; i < num_signals; i++)
    if (num == signals_[i].num_)
      return &signals_[i];

  return nullptr;
}

int
CwshSignal::
getNumSignals()
{
  return sizeof(signals_)/sizeof(CwshSignal);
}

CwshSignal *
CwshSignal::
getSignal(int i)
{
  return &signals_[i];
}
