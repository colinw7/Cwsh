#include <CwshI.h>

namespace Cwsh {

DirStack::
DirStack()
{
}

void
DirStack::
push()
{
  dirStack_.push_back(COSFile::getCurrentDir());
}

void
DirStack::
push(const std::string &dirname)
{
  dirStack_.push_back(dirname);
}

std::string
DirStack::
pop()
{
  if (dirStack_.size() == 0) {
    CTHROW("Directory Stack underflow.");
    return COSFile::getCurrentDir();
  }

  std::string dirname = dirStack_[dirStack_.size() - 1];

  dirStack_.pop_back();

  return dirname;
}

std::string
DirStack::
pop(int pos)
{
  int num_dirs = int(dirStack_.size());

  if (num_dirs < pos) {
    CTHROW("Directory Stack underflow.");
    return COSFile::getCurrentDir();
  }

  std::string dirname = dirStack_[num_dirs - pos - 1];

  for (int i = num_dirs - pos - 1; i < num_dirs - 2; ++i)
    dirStack_[i] = dirStack_[i + 1];

  dirStack_.pop_back();

  return dirname;
}

int
DirStack::
size()
{
  return int(dirStack_.size());
}

void
DirStack::
print(std::ostream &os)
{
  std::string dirname = COSFile::getCurrentDir();

  os << String::replaceHome(dirname);

  int num_dirs = int(dirStack_.size());

  for (int i = num_dirs - 1; i >= 0; i--) {
    os << " ";

    os << String::replaceHome(dirStack_[i]);
  }

  os << "\n";
}

}
