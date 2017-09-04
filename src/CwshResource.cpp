#include <CwshI.h>
#include <COSLimit.h>

enum class CwshResourceType {
  NONE,
  TIME,
  SIZE,
  DATA,
};

struct CwshResourceLimit {
  std::string              name;
  COSLimit::LimitSetProc   setProc;
  COSLimit::LimitGetProc   getProc;
  COSLimit::LimitUnsetProc unsetProc;
  CwshResourceType         type;
};

#define limitProcs(p) \
  COSLimit::set##p##Limit, COSLimit::get##p##Limit, COSLimit::unset##p##Limit

CwshResourceLimit
CwshResource::limits_[] = {
  { "cputime"     , limitProcs(CPU         ) , CwshResourceType::TIME, },
  { "filesize"    , limitProcs(FileSize    ) , CwshResourceType::SIZE, },
  { "datasize"    , limitProcs(DataSize    ) , CwshResourceType::SIZE, },
  { "stacksize"   , limitProcs(StackSize   ) , CwshResourceType::SIZE, },
  { "coredumpsize", limitProcs(CoreDumpSize) , CwshResourceType::SIZE, },
  { "memoryuse"   , limitProcs(MemoryUse   ) , CwshResourceType::SIZE, },
  { "vmemoryuse"  , limitProcs(VMemoryUse  ) , CwshResourceType::SIZE, },
  { "descriptors" , limitProcs(Descriptors ) , CwshResourceType::DATA, },
  { "memorylocked", limitProcs(MemoryLocked) , CwshResourceType::SIZE, },
  { "maxproc"     , limitProcs(MaxProc     ) , CwshResourceType::DATA, },
  { "openfiles"   , limitProcs(OpenFiles   ) , CwshResourceType::DATA, },
  { "addressspace", limitProcs(AddressSpace) , CwshResourceType::SIZE, },
  { ""            , nullptr, nullptr, nullptr, CwshResourceType::NONE, },
};

CwshResource::
CwshResource()
{
}

void
CwshResource::
limit(const std::string &name, const std::string &value, bool hard)
{
  CwshResourceLimit *rlimit = getLimit(name);

  if (! rlimit)
    CWSH_THROW("No such limit.");

  int ivalue = convertValue(rlimit, value);

  if (! (*rlimit->setProc)(ivalue, hard))
    CWSH_THROW(name + ": Can't get limit.");
}

void
CwshResource::
unlimitAll()
{
  for (int i = 0; limits_[i].type != CwshResourceType::NONE; i++)
    (*limits_[i].unsetProc)();
}

void
CwshResource::
unlimit(const std::string &name)
{
  CwshResourceLimit *rlimit = getLimit(name);

  if (! rlimit)
    CWSH_THROW("No such limit.");

  (*rlimit->unsetProc)();
}

void
CwshResource::
printAll(bool hard)
{
  for (int i = 0; limits_[i].type != CwshResourceType::NONE; i++)
    print(&limits_[i], hard);
}

void
CwshResource::
print(const std::string &name, bool hard)
{
  CwshResourceLimit *rlimit = getLimit(name);

  if (! rlimit)
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

  std::cout << rlimit->name;

  int name_len = rlimit->name.size();

  for (int i = name_len; i < 16; i++)
    std::cout << " ";

  if (COSLimit::isLimitValueInfinity(value)) {
    std::cout << "unlimited" << std::endl;
    return;
  }

  if      (rlimit->type == CwshResourceType::TIME) {
    int value1 = value/3600;
    int value2 = (value - value1*3600)/60;
    int value3 = value - value1*3600 - value2*60;

    if (value1 > 0)
      std::cout << value1 << value2 << value3 << std::endl;
    else
      std::cout << value2 << value3 << std::endl;
  }
  else if (rlimit->type == CwshResourceType::SIZE)
    std::cout << (int) (value/1024) << " kbytes" << std::endl;
  else
    std::cout << value << std::endl;
}

int
CwshResource::
convertValue(CwshResourceLimit *rlimit, const std::string &value)
{
  uint len = value.size();

  if (len == 0 || ! isdigit(value[0]))
    CWSH_THROW("Invalid Value.");

  uint i = 0;

  int ivalue;

  if (! CStrUtil::readInteger(value, &i, &ivalue))
    CWSH_THROW("Invalid Value.");

  if      (rlimit->type == CwshResourceType::TIME) {
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
  else if (rlimit->type == CwshResourceType::SIZE) {
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
getLimit(const std::string &name)
{
  for (int i = 0; limits_[i].type != CwshResourceType::NONE; i++)
    if (limits_[i].name == name)
      return &limits_[i];

  CWSH_THROW("Bad Resource Name " + name);
}
