#include <COSLimit.h>

#ifndef RLIM_INFINITY
# define RLIM_INFINITY -1
#endif

#ifdef OS_LINUX
typedef __rlimit_resource LimitId;
#else
typedef int LimitId;
#endif

//------------

static bool
setLimit(LimitId id, COSLimit::LimitVal val, bool hard)
{
  struct rlimit limit;

  int error = getrlimit(id, &limit);

  if (error != 0)
    return false;

  if (hard)
    limit.rlim_max = val;
  else
    limit.rlim_cur = val;

  error = setrlimit(id, &limit);

  if (error != 0)
    return false;

  return true;
}

static bool
getLimit(LimitId id, COSLimit::LimitVal *val, bool hard)
{
  struct rlimit limit;

  int error = getrlimit(id, &limit);

  if (error != 0)
    return false;

  if (hard)
    *val = limit.rlim_max;
  else
    *val = limit.rlim_cur;

  return true;
}

static bool
unsetLimit(LimitId id)
{
  struct rlimit limit;

  limit.rlim_max = RLIM_INFINITY;
  limit.rlim_cur = RLIM_INFINITY;

  int error = setrlimit(id, &limit);

  if (error != 0)
    return false;

  return true;
}

//------------

bool
COSLimit::
setCPULimit(COSLimit::LimitVal val, bool hard)
{
#ifdef RLIMIT_CPU
  return setLimit(RLIMIT_CPU, val, hard);
#else
  return false;
#endif
}

bool
COSLimit::
getCPULimit(COSLimit::LimitVal *val, bool hard)
{
#ifdef RLIMIT_CPU
  return getLimit(RLIMIT_CPU, val, hard);
#else
  return false;
#endif
}

bool
COSLimit::
unsetCPULimit()
{
#ifdef RLIMIT_CPU
  return unsetLimit(RLIMIT_CPU);
#else
  return false;
#endif
}

//------------

bool
COSLimit::
setFileSizeLimit(COSLimit::LimitVal val, bool hard)
{
#ifdef RLIMIT_FSIZE
  return setLimit(RLIMIT_FSIZE, val, hard);
#else
  return false;
#endif
}

bool
COSLimit::
getFileSizeLimit(COSLimit::LimitVal *val, bool hard)
{
#ifdef RLIMIT_FSIZE
  return getLimit(RLIMIT_FSIZE, val, hard);
#else
  return false;
#endif
}

bool
COSLimit::
unsetFileSizeLimit()
{
#ifdef RLIMIT_FSIZE
  return unsetLimit(RLIMIT_FSIZE);
#else
  return false;
#endif
}

//------------

bool
COSLimit::
setDataSizeLimit(COSLimit::LimitVal val, bool hard)
{
#ifdef RLIMIT_DATA
  return setLimit(RLIMIT_DATA, val, hard);
#else
  return false;
#endif
}

bool
COSLimit::
getDataSizeLimit(COSLimit::LimitVal *val, bool hard)
{
#ifdef RLIMIT_DATA
  return getLimit(RLIMIT_DATA, val, hard);
#else
  return false;
#endif
}

bool
COSLimit::
unsetDataSizeLimit()
{
#ifdef RLIMIT_DATA
  return unsetLimit(RLIMIT_DATA);
#else
  return false;
#endif
}

//------------

bool
COSLimit::
setStackSizeLimit(COSLimit::LimitVal val, bool hard)
{
#ifdef RLIMIT_STACK
  return setLimit(RLIMIT_STACK, val, hard);
#else
  return false;
#endif
}

bool
COSLimit::
getStackSizeLimit(COSLimit::LimitVal *val, bool hard)
{
#ifdef RLIMIT_STACK
  return getLimit(RLIMIT_STACK, val, hard);
#else
  return false;
#endif
}

bool
COSLimit::
unsetStackSizeLimit()
{
#ifdef RLIMIT_STACK
  return unsetLimit(RLIMIT_STACK);
#else
  return false;
#endif
}

//------------

bool
COSLimit::
setCoreDumpSizeLimit(COSLimit::LimitVal val, bool hard)
{
#ifdef RLIMIT_CORE
  return setLimit(RLIMIT_CORE, val, hard);
#else
  return false;
#endif
}

bool
COSLimit::
getCoreDumpSizeLimit(COSLimit::LimitVal *val, bool hard)
{
#ifdef RLIMIT_CORE
  return getLimit(RLIMIT_CORE, val, hard);
#else
  return false;
#endif
}

bool
COSLimit::
unsetCoreDumpSizeLimit()
{
#ifdef RLIMIT_CORE
  return unsetLimit(RLIMIT_CORE);
#else
  return false;
#endif
}

//------------

bool
COSLimit::
setMemoryUseLimit(COSLimit::LimitVal val, bool hard)
{
#ifdef RLIMIT_RSS
  return setLimit(RLIMIT_RSS, val, hard);
#else
  return false;
#endif
}

bool
COSLimit::
getMemoryUseLimit(COSLimit::LimitVal *val, bool hard)
{
#ifdef RLIMIT_RSS
  return getLimit(RLIMIT_RSS, val, hard);
#else
  return false;
#endif
}

bool
COSLimit::
unsetMemoryUseLimit()
{
#ifdef RLIMIT_RSS
  return unsetLimit(RLIMIT_RSS);
#else
  return false;
#endif
}

//------------

bool
COSLimit::
setVMemoryUseLimit(COSLimit::LimitVal val, bool hard)
{
#ifdef RLIMIT_AS
  return setLimit(RLIMIT_AS, val, hard);
#else
  return false;
#endif
}

bool
COSLimit::
getVMemoryUseLimit(COSLimit::LimitVal *val, bool hard)
{
#ifdef RLIMIT_AS
  return getLimit(RLIMIT_AS, val, hard);
#else
  return false;
#endif
}

bool
COSLimit::
unsetVMemoryUseLimit()
{
#ifdef RLIMIT_AS
  return unsetLimit(RLIMIT_AS);
#else
  return false;
#endif
}

//------------

bool
COSLimit::
setDescriptorsLimit(COSLimit::LimitVal val, bool hard)
{
#ifdef RLIMIT_NOFILE
  return setLimit(RLIMIT_NOFILE, val, hard);
#else
  return false;
#endif
}

bool
COSLimit::
getDescriptorsLimit(COSLimit::LimitVal *val, bool hard)
{
#ifdef RLIMIT_NOFILE
  return getLimit(RLIMIT_NOFILE, val, hard);
#else
  return false;
#endif
}

bool
COSLimit::
unsetDescriptorsLimit()
{
#ifdef RLIMIT_NOFILE
  return unsetLimit(RLIMIT_NOFILE);
#else
  return false;
#endif
}

//------------

bool
COSLimit::
setMemoryLockedLimit(COSLimit::LimitVal val, bool hard)
{
#ifdef RLIMIT_MEMLOCK
  return setLimit(RLIMIT_MEMLOCK, val, hard);
#else
  return false;
#endif
}

bool
COSLimit::
getMemoryLockedLimit(COSLimit::LimitVal *val, bool hard)
{
#ifdef RLIMIT_MEMLOCK
  return getLimit(RLIMIT_MEMLOCK, val, hard);
#else
  return false;
#endif
}

bool
COSLimit::
unsetMemoryLockedLimit()
{
#ifdef RLIMIT_MEMLOCK
  return unsetLimit(RLIMIT_MEMLOCK);
#else
  return false;
#endif
}

//------------

bool
COSLimit::
setMaxProcLimit(COSLimit::LimitVal val, bool hard)
{
#ifdef RLIMIT_NPROC
  return setLimit(RLIMIT_NPROC, val, hard);
#else
  return false;
#endif
}

bool
COSLimit::
getMaxProcLimit(COSLimit::LimitVal *val, bool hard)
{
#ifdef RLIMIT_NPROC
  return getLimit(RLIMIT_NPROC, val, hard);
#else
  return false;
#endif
}

bool
COSLimit::
unsetMaxProcLimit()
{
#ifdef RLIMIT_NPROC
  return unsetLimit(RLIMIT_NPROC);
#else
  return false;
#endif
}

//------------

bool
COSLimit::
setOpenFilesLimit(COSLimit::LimitVal val, bool hard)
{
#ifdef RLIMIT_NOFILE
  return setLimit(RLIMIT_NOFILE, val, hard);
#else
  return false;
#endif
}

bool
COSLimit::
getOpenFilesLimit(COSLimit::LimitVal *val, bool hard)
{
#ifdef RLIMIT_NOFILE
  return getLimit(RLIMIT_NOFILE, val, hard);
#else
  return false;
#endif
}

bool
COSLimit::
unsetOpenFilesLimit()
{
#ifdef RLIMIT_NOFILE
  return unsetLimit(RLIMIT_NOFILE);
#else
  return false;
#endif
}

//------------

bool
COSLimit::
setAddressSpaceLimit(COSLimit::LimitVal val, bool hard)
{
#ifdef RLIMIT_AS
  return setLimit(RLIMIT_AS, val, hard);
#else
  return false;
#endif
}

bool
COSLimit::
getAddressSpaceLimit(COSLimit::LimitVal *val, bool hard)
{
#ifdef RLIMIT_AS
  return getLimit(RLIMIT_AS, val, hard);
#else
  return false;
#endif
}

bool
COSLimit::
unsetAddressSpaceLimit()
{
#ifdef RLIMIT_AS
  return unsetLimit(RLIMIT_AS);
#else
  return false;
#endif
}

//------------

bool
COSLimit::
isLimitValueInfinity(COSLimit::LimitVal val)
{
  return (val == RLIM_INFINITY);
}
