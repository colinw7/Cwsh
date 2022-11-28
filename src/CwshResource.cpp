#include <CwshI.h>
#include <COSLimit.h>

namespace Cwsh {

enum class ResourceType {
  NONE,
  TIME,
  SIZE,
  DATA,
};

struct ResourceLimit {
  std::string              name;
  COSLimit::LimitSetProc   setProc;
  COSLimit::LimitGetProc   getProc;
  COSLimit::LimitUnsetProc unsetProc;
  ResourceType             type;
};

#define limitProcs(p) \
  COSLimit::set##p##Limit, COSLimit::get##p##Limit, COSLimit::unset##p##Limit

ResourceLimit
Resource::limits_[] = {
  { "cputime"     , limitProcs(CPU         ) , ResourceType::TIME, },
  { "filesize"    , limitProcs(FileSize    ) , ResourceType::SIZE, },
  { "datasize"    , limitProcs(DataSize    ) , ResourceType::SIZE, },
  { "stacksize"   , limitProcs(StackSize   ) , ResourceType::SIZE, },
  { "coredumpsize", limitProcs(CoreDumpSize) , ResourceType::SIZE, },
  { "memoryuse"   , limitProcs(MemoryUse   ) , ResourceType::SIZE, },
  { "vmemoryuse"  , limitProcs(VMemoryUse  ) , ResourceType::SIZE, },
  { "descriptors" , limitProcs(Descriptors ) , ResourceType::DATA, },
  { "memorylocked", limitProcs(MemoryLocked) , ResourceType::SIZE, },
  { "maxproc"     , limitProcs(MaxProc     ) , ResourceType::DATA, },
  { "openfiles"   , limitProcs(OpenFiles   ) , ResourceType::DATA, },
  { "addressspace", limitProcs(AddressSpace) , ResourceType::SIZE, },
  { ""            , nullptr, nullptr, nullptr, ResourceType::NONE, },
};

Resource::
Resource()
{
}

void
Resource::
limit(const std::string &name, const std::string &value, bool hard)
{
  auto *rlimit = getLimit(name);

  if (! rlimit)
    CWSH_THROW("No such limit.");

  int ivalue = convertValue(rlimit, value);

  if (! (*rlimit->setProc)(ivalue, hard))
    CWSH_THROW(name + ": Can't get limit.");
}

void
Resource::
unlimitAll()
{
  for (int i = 0; limits_[i].type != ResourceType::NONE; i++)
    (*limits_[i].unsetProc)();
}

void
Resource::
unlimit(const std::string &name)
{
  auto *rlimit = getLimit(name);

  if (! rlimit)
    CWSH_THROW("No such limit.");

  (*rlimit->unsetProc)();
}

void
Resource::
printAll(bool hard)
{
  for (int i = 0; limits_[i].type != ResourceType::NONE; i++)
    print(&limits_[i], hard);
}

void
Resource::
print(const std::string &name, bool hard)
{
  auto *rlimit = getLimit(name);

  if (! rlimit)
    CWSH_THROW("No such limit.");

  print(rlimit, hard);
}

void
Resource::
print(ResourceLimit *rlimit, bool hard)
{
  COSLimit::LimitVal value;

  if (! (*rlimit->getProc)(&value, hard))
    CWSH_THROW(rlimit->name + ": Can't get limit.");

  std::cout << rlimit->name;

  int name_len = int(rlimit->name.size());

  for (int i = name_len; i < 16; i++)
    std::cout << " ";

  if (COSLimit::isLimitValueInfinity(value)) {
    std::cout << "unlimited\n";
    return;
  }

  if      (rlimit->type == ResourceType::TIME) {
    int value1 = int(value/3600);
    int value2 = int((value - value1*3600)/60);
    int value3 = int(value - value1*3600 - value2*60);

    if (value1 > 0)
      std::cout << value1 << value2 << value3 << "\n";
    else
      std::cout << value2 << value3 << "\n";
  }
  else if (rlimit->type == ResourceType::SIZE)
    std::cout << int(value/1024) << " kbytes\n";
  else
    std::cout << value << "\n";
}

int
Resource::
convertValue(ResourceLimit *rlimit, const std::string &value)
{
  uint len = uint(value.size());

  if (len == 0 || ! isdigit(value[0]))
    CWSH_THROW("Invalid Value.");

  uint i = 0;

  int ivalue;

  if (! CStrUtil::readInteger(value, &i, &ivalue))
    CWSH_THROW("Invalid Value.");

  if      (rlimit->type == ResourceType::TIME) {
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
  else if (rlimit->type == ResourceType::SIZE) {
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

ResourceLimit *
Resource::
getLimit(const std::string &name)
{
  for (int i = 0; limits_[i].type != ResourceType::NONE; i++)
    if (limits_[i].name == name)
      return &limits_[i];

  CWSH_THROW("Bad Resource Name " + name);
}

}
