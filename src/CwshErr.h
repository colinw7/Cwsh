#ifndef CWSH_ERR_H
#define CWSH_ERR_H

#include <iostream>

#define CWSH_THROW(message) \
  throw new Cwsh::Err((message),__FILE__,__LINE__)

#define CWSH_THROWQ(qualifier, message) \
  throw new Cwsh::Err((qualifier),(message),__FILE__,__LINE__)

namespace Cwsh {

struct Err {
  std::string qualifier;
  std::string message;
  std::string file;
  int         line;

  Err(const std::string &qualifier1, const std::string &message1,
      const std::string &file1, int line1) :
    qualifier(qualifier1), message(message1), file(file1), line(line1) {
  }

  Err(const std::string &message1, const std::string &file1, int line1) :
    qualifier(""), message(message1), file(file1), line(line1) {
  }

  void print() {
    if (file != "")
      std::cerr << "[" << file << ":" << line << "] ";

    if (qualifier != "")
      std::cerr << qualifier << ": " << message << "\n";
    else
      std::cerr << message << "\n";
  }
};

}

#endif
