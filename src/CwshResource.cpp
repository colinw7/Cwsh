#include "CwshI.h"
#include <COSLimit.h>

enum CwshResourceType {
  CWSH_RESOURCE_TYPE_NONE,
  CWSH_RESOURCE_TYPE_TIME,
  CWSH_RESOURCE_TYPE_SIZE,
  CWSH_RESOURCE_TYPE_DATA,
};

struct CwshResourceLimit {
  string                   name;
  COSLimit::LimitSetProc   setProc;
  COSLimit::LimitGetProc   getProc;
  COSLimit::LimitUnsetProc unsetProc;
  CwshResourceType         type;
};

#define limitProcs(p) \
  COSLimit::set##p##Limit, COSLimit::get##p##Limit, COSLimit::unset##p##Limit

CwshResourceLimit
CwshResource::limits_[] = {
  { "cputime"     , limitProcs(CPU         ), CWSH_RESOURCE_TYPE_TIME, },
  { "filesize"    , limitProcs(FileSize    ), CWSH_RESOURCE_TYPE_SIZE, },
  { "datasize"    , limitProcs(DataSize    ), CWSH_RESOURCE_TYPE_SIZE, },
  { "stacksize"   , limitProcs(StackSize   ), CWSH_RESOURCE_TYPE_SIZE, },
  { "coredumpsize", limitProcs(CoreDumpSize), CWSH_RESOURCE_TYPE_SIZE, },
  { "memoryuse"   , limitProcs(MemoryUse   ), CWSH_RESOURCE_TYPE_SIZE, },
  { "vmemoryuse"  , limitProcs(VMemoryUse  ), CWSH_RESOURCE_TYPE_SIZE, },
  { "descriptors" , limitProcs(Descriptors ), CWSH_RESOURCE_TYPE_DATA, },
  { "memorylocked", limitProcs(MemoryLocked), CWSH_RESOURCE_TYPE_SIZE, },
  { "maxproc"     , limitProcs(MaxProc     ), CWSH_RESOURCE_TYPE_DATA, },
  { "openfiles"   , limitProcs(OpenFiles   ), CWSH_RESOURCE_TYPE_DATA, },
  { "addressspace", limitProcs(AddressSpace), CWSH_RESOURCE_TYPE_SIZE, },
  { ""            , NULL, NULL, NULL        , CWSH_RESOURCE_TYPE_NONE, },
};

CwshResource::
CwshResource()
{
}

void
CwshResource::
limit(const string &name, const string &value, bool hard)
{
  CwshResourceLimit *rlimit = getLimit(name);

  if (rlimit == NULL)
    CWSH_THROW("No such limit.");

  int ivalue = convertValue(rlimit, value);

  if (! (*rlimit->setProc)(ivalue, hard))
    CWSH_THROW(name + ": Can't get limit.");
}

void
CwshResource::
unlimitAll()
{
  for (int i = 0; limits_[i].type != CWSH_RESOURCE_TYPE_NONE; i++)
    (*limits_[i].unsetProc)();
}

void
CwshResource::
unlimit(const string &name)
{
  CwshResourceLimit *rlimit = getLimit(name);

  if (rlimit == NULL)
    CWSH_THROW("No such limit.");

  (*rlimit->unsetProc)();
}

void
CwshResource::
printAll(bool hard)
{
  for (int i = 0; limits_[i].type != CWSH_RESOURCE_TYPE_NONE; i++)
    print(&limits_[i], hard);
}

void
CwshResource::
print(const string &name, bool hard)
{
  CwshResourceLimit *rlimit = getLimit(name);

  if (rlimit == NULL)
    CWSH_THROW("No such limit.");

  print(rlimit, hard);
}

void
CwshResource::
print(CwshResourceLimit *rlimit, bool hard)
{
  COSLimit::LimitVal value;

  if (! (*rlimit->getProc)(&value, hard))
    CWSH_THROW(rlimit->name + ": Can't get limit.");

  cout << rlimit->name;

  int name_len = rlimit->name.size();

  for (int i = name_len; i < 16; i++)
    cout << " ";

  if (COSLimit::isLimitValueInfinity(value)) {
    cout << "unlimited" << endl;
    return;
  }

  if      (rlimit->type == CWSH_RESOURCE_TYPE_TIME) {
    int value1 = value/3600;
    int value2 = (value - value1*3600)/60;
    int value3 = value - value1*3600 - value2*60;

    if (value1 > 0)
      cout << value1 << value2 << value3 << endl;
    else
      cout << value2 << value3 << endl;
  }
  else if (rlimit->type == CWSH_RESOURCE_TYPE_SIZE)
    cout << (int) (value/1024) << " kbytes" << endl;
  else
    cout << value << endl;
}

int
CwshResource::
convertValue(CwshResourceLimit *rlimit, const string &value)
{
  uint len = value.size();

  if (len == 0 || ! isdigit(value[0]))
    CWSH_THROW("Invalid Value.");

  uint i = 0;

  int ivalue;

  if (! CStrUtil::readInteger(value, &i, &ivalue))
    CWSH_THROW("Invalid Value.");

  if      (rlimit->type == CWSH_RESOURCE_TYPE_TIME) {
    if      (i < len && value[i] == 'h') {
      i++;

      ivalue *= 3600;
    }
    else if (i < len && value[i] == 'm') {
      i++;

      ivalue *= 60;
    }
    else if (i < len && value[i] == ':') {
      i++;

      ivalue *= 60;

      int ivalue1;

      if (! CStrUtil::readInteger(value, &i, &ivalue1))
        CWSH_THROW("Invalid Value.");

      ivalue += ivalue1;
    }
  }
  else if (rlimit->type == CWSH_RESOURCE_TYPE_SIZE) {
    if      (i < len && value[i] == 'k') {
      i++;

      ivalue <<= 10;
    }
    else if (i < len && value[i] == 'm') {
      i++;

      ivalue <<= 20;
    }
    else
      ivalue <<= 10;
  }

  if (i != len)
    CWSH_THROW("Invalid Value.");

  return ivalue;
}

CwshResourceLimit *
CwshResource::
getLimit(const string &name)
{
  for (int i = 0; limits_[i].type != CWSH_RESOURCE_TYPE_NONE; i++)
    if (limits_[i].name == name)
      return &limits_[i];

  CWSH_THROW("Bad Resource Name " + name);
}
