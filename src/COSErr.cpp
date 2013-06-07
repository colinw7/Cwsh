#include <COSErr.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

static const char *ename[] = {
    /*   0 */ "",
    /*   1 */ "EPERM", "ENOENT", "ESRCH", "EINTR", "EIO", "ENXIO",
    /*   7 */ "E2BIG", "ENOEXEC", "EBADF", "ECHILD",
    /*  11 */ "EAGAIN/EWOULDBLOCK", "ENOMEM", "EACCES", "EFAULT",
    /*  15 */ "ENOTBLK", "EBUSY", "EEXIST", "EXDEV", "ENODEV",
    /*  20 */ "ENOTDIR", "EISDIR", "EINVAL", "ENFILE", "EMFILE",
    /*  25 */ "ENOTTY", "ETXTBSY", "EFBIG", "ENOSPC", "ESPIPE",
    /*  30 */ "EROFS", "EMLINK", "EPIPE", "EDOM", "ERANGE",
    /*  35 */ "EDEADLK/EDEADLOCK", "ENAMETOOLONG", "ENOLCK", "ENOSYS",
    /*  39 */ "ENOTEMPTY", "ELOOP", "", "ENOMSG", "EIDRM", "ECHRNG",
    /*  45 */ "EL2NSYNC", "EL3HLT", "EL3RST", "ELNRNG", "EUNATCH",
    /*  50 */ "ENOCSI", "EL2HLT", "EBADE", "EBADR", "EXFULL", "ENOANO",
    /*  56 */ "EBADRQC", "EBADSLT", "", "EBFONT", "ENOSTR", "ENODATA",
    /*  62 */ "ETIME", "ENOSR", "ENONET", "ENOPKG", "EREMOTE",
    /*  67 */ "ENOLINK", "EADV", "ESRMNT", "ECOMM", "EPROTO",
    /*  72 */ "EMULTIHOP", "EDOTDOT", "EBADMSG", "EOVERFLOW",
    /*  76 */ "ENOTUNIQ", "EBADFD", "EREMCHG", "ELIBACC", "ELIBBAD",
    /*  81 */ "ELIBSCN", "ELIBMAX", "ELIBEXEC", "EILSEQ", "ERESTART",
    /*  86 */ "ESTRPIPE", "EUSERS", "ENOTSOCK", "EDESTADDRREQ",
    /*  90 */ "EMSGSIZE", "EPROTOTYPE", "ENOPROTOOPT",
    /*  93 */ "EPROTONOSUPPORT", "ESOCKTNOSUPPORT",
    /*  95 */ "EOPNOTSUPP/ENOTSUP", "EPFNOSUPPORT", "EAFNOSUPPORT",
    /*  98 */ "EADDRINUSE", "EADDRNOTAVAIL", "ENETDOWN", "ENETUNREACH",
    /* 102 */ "ENETRESET", "ECONNABORTED", "ECONNRESET", "ENOBUFS",
    /* 106 */ "EISCONN", "ENOTCONN", "ESHUTDOWN", "ETOOMANYREFS",
    /* 110 */ "ETIMEDOUT", "ECONNREFUSED", "EHOSTDOWN", "EHOSTUNREACH",
    /* 114 */ "EALREADY", "EINPROGRESS", "ESTALE", "EUCLEAN",
    /* 118 */ "ENOTNAM", "ENAVAIL", "EISNAM", "EREMOTEIO", "EDQUOT",
    /* 123 */ "ENOMEDIUM", "EMEDIUMTYPE", "ECANCELED", "ENOKEY",
    /* 127 */ "EKEYEXPIRED", "EKEYREVOKED", "EKEYREJECTED",
    /* 130 */ "EOWNERDEAD", "ENOTRECOVERABLE", "ERFKILL"
};

#define MAX_ENAME 132

void
COSErr::
exit(const char *format, ...)
{
  va_list argList;

  va_start(argList, format);

  outputError(true, errno, true, format, argList);

  va_end(argList);

  terminate(true);
}

/* Display error message including 'errno' diagnostic, and
   return to caller */
void
COSErr::
message(const char *format, ...)
{
  va_list argList;

  int savedErrno = errno;       /* In case we change it here */

  va_start(argList, format);

  outputError(true, errno, true, format, argList);

  va_end(argList);

  errno = savedErrno;
}

void
COSErr::
outputError(bool useErr, int err, bool flushStdout, const char *format, va_list ap)
{
  enum { BUF_SIZE = 500 };

  char buf[BUF_SIZE], userMsg[BUF_SIZE], errText[BUF_SIZE];

  vsnprintf(userMsg, BUF_SIZE, format, ap);

  if (useErr)
    snprintf(errText, BUF_SIZE, " [%s %s]", (err > 0 && err <= MAX_ENAME) ?
             ename[err] : "?UNKNOWN?", strerror(err));
  else
    snprintf(errText, BUF_SIZE, ":");

  snprintf(buf, BUF_SIZE, "ERROR%s %s\n", errText, userMsg);

  if (flushStdout)
    fflush(stdout); /* Flush any pending stdout */

  fputs(buf, stderr);

  fflush(stderr); /* In case stderr is not line-buffered */
}

void
COSErr::
terminate(bool useExit3)
{
  /* Dump core if EF_DUMPCORE environment variable is defined and is a nonempty string;
     otherwise call exit(3) or _exit(2), depending on the value of 'useExit3'. */

  char *s = getenv("EF_DUMPCORE");

  if      (s != NULL && *s != '\0')
    abort();
  else if (useExit3)
    ::exit(EXIT_FAILURE);
  else
    _exit(EXIT_FAILURE);
}
