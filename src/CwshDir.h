#ifndef CWSH_DIR_H
#define CWSH_DIR_H

namespace CwshDir {
  std::string lookup(Cwsh *cwsh, const std::string &dirname, bool required=true);
}

#endif
