#ifndef CWSH_SH_MEM_H
#define CWSH_SH_MEM_H

#ifdef USE_SHM

#include <sys/param.h>

#include <CShMem.h>

namespace Cwsh {

struct ShMemData {
  char path[MAXPATHLEN + 1];
};

class ShMem : public CShMem {
 public:
  ShMem() :
   CShMem("Cwsh") {
  }

  bool getPath(char *path) {
    bool flag = read(&data_);

    if (flag)
      strcpy(path, data_.path);

    return flag;
  }

  bool setPath(const char *path) {
    strcpy(data_.path, path);

    return write(&data_);
  }

  uint getDataSize() const { return sizeof(data_); }

 private:
  ShMemData data_;
};

}

#endif

#endif
