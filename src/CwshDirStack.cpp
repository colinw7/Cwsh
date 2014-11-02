#include <CwshI.h>

CwshDirStack::
CwshDirStack()
{
}

void
CwshDirStack::
push()
{
  dir_stack_.push_back(COSFile::getCurrentDir());
}

void
CwshDirStack::
push(const string &dirname)
{
  dir_stack_.push_back(dirname);
}

string
CwshDirStack::
pop()
{
  if (dir_stack_.size() == 0) {
    CTHROW("Directory Stack underflow.");
    return COSFile::getCurrentDir();
  }

  string dirname = dir_stack_[dir_stack_.size() - 1];

  dir_stack_.pop_back();

  return dirname;
}

string
CwshDirStack::
pop(int pos)
{
  int num_dirs = dir_stack_.size();

  if (num_dirs < pos) {
    CTHROW("Directory Stack underflow.");
    return COSFile::getCurrentDir();
  }

  string dirname = dir_stack_[num_dirs - pos - 1];

  for (int i = num_dirs - pos - 1; i < num_dirs - 2; ++i)
    dir_stack_[i] = dir_stack_[i + 1];

  dir_stack_.pop_back();

  return dirname;
}

int
CwshDirStack::
size()
{
  return dir_stack_.size();
}

void
CwshDirStack::
print(ostream &os)
{
  string dirname = COSFile::getCurrentDir();

  os << CwshString::replaceHome(dirname);

  int num_dirs = dir_stack_.size();

  for (int i = num_dirs - 1; i >= 0; i--) {
    os << " ";

    os << CwshString::replaceHome(dir_stack_[i]);
  }

  os << std::endl;
}
