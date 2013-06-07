#ifndef COS_ERR_H
#define COS_ERR_H

#include <cstdarg>

namespace COSErr {
  void exit(const char *format, ...);

  void message(const char *format, ...);

  void outputError(bool useErr, int err, bool flushStdout, const char *format, va_list ap);

  void terminate(bool useExit3);
}

#endif
