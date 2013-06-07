#ifndef COSLimit_H
#define COSLimit_H

#include <sys/resource.h>

namespace COSLimit {
  typedef rlim_t LimitVal;

  typedef bool (*LimitSetProc)(LimitVal val, bool hard);
  typedef bool (*LimitGetProc)(LimitVal *val, bool hard);
  typedef bool (*LimitUnsetProc)();

  bool setCPULimit(LimitVal val, bool hard);
  bool getCPULimit(LimitVal *val, bool hard);
  bool unsetCPULimit();

  bool setFileSizeLimit(LimitVal val, bool hard);
  bool getFileSizeLimit(LimitVal *val, bool hard);
  bool unsetFileSizeLimit();

  bool setDataSizeLimit(LimitVal val, bool hard);
  bool getDataSizeLimit(LimitVal *val, bool hard);
  bool unsetDataSizeLimit();

  bool setStackSizeLimit(LimitVal val, bool hard);
  bool getStackSizeLimit(LimitVal *val, bool hard);
  bool unsetStackSizeLimit();

  bool setCoreDumpSizeLimit(LimitVal val, bool hard);
  bool getCoreDumpSizeLimit(LimitVal *val, bool hard);
  bool unsetCoreDumpSizeLimit();

  bool setMemoryUseLimit(LimitVal val, bool hard);
  bool getMemoryUseLimit(LimitVal *val, bool hard);
  bool unsetMemoryUseLimit();

  bool setVMemoryUseLimit(LimitVal val, bool hard);
  bool getVMemoryUseLimit(LimitVal *val, bool hard);
  bool unsetVMemoryUseLimit();

  bool setDescriptorsLimit(LimitVal val, bool hard);
  bool getDescriptorsLimit(LimitVal *val, bool hard);
  bool unsetDescriptorsLimit();

  bool setMemoryLockedLimit(LimitVal val, bool hard);
  bool getMemoryLockedLimit(LimitVal *val, bool hard);
  bool unsetMemoryLockedLimit();

  bool setMaxProcLimit(LimitVal val, bool hard);
  bool getMaxProcLimit(LimitVal *val, bool hard);
  bool unsetMaxProcLimit();

  bool setOpenFilesLimit(LimitVal val, bool hard);
  bool getOpenFilesLimit(LimitVal *val, bool hard);
  bool unsetOpenFilesLimit();

  bool setAddressSpaceLimit(LimitVal val, bool hard);
  bool getAddressSpaceLimit(LimitVal *val, bool hard);
  bool unsetAddressSpaceLimit();

  bool isLimitValueInfinity(LimitVal val);
}

#endif
