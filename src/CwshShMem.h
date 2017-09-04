#ifndef CWSH_SH_MEM_H
#define CWSH_SH_MEM_H

#ifdef USE_SHM

#include <sys/param.h>

#include <CShMem.h>

struct CwshShMemData {
  char path[MAXPATHLEN + 1];
};

class CwshShMem : public CShMem {
 public:
  CwshShMem() :
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
  CwshShMemData data_;
};

#endif

#endif
