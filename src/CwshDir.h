#ifndef CWSH_DIR_H
#define CWSH_DIR_H

namespace Cwsh {

namespace Dir {
  std::string lookup(App *cwsh, const std::string &dirname, bool required=true);
}

}

#endif
