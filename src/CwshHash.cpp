#include <CwshI.h>

namespace Cwsh {

Hash::
Hash(App *cwsh) :
 cwsh_(cwsh)
{
}

void
Hash::
addFilePath(const std::string &filename, const std::string &path)
{
  if (filePathActive_) {
    filePathMap_[filename] = path;

    if (cwsh_->getDebug())
      std::cerr << "Added (" << filename << "," << path << ") to hash.\n";
  }
}

std::string
Hash::
getFilePath(const std::string &filename)
{
  if (filePathActive_) {
    auto p = filePathMap_.find(filename);

    if (p != filePathMap_.end()) {
      if (cwsh_->getDebug())
        std::cerr << "Found (" << filename << "," << (*p).second << ") in hash.\n";

      return (*p).second;
    }

    return "";
  }
  else
    return "";
}

void
Hash::
clearFilePath()
{
  filePathMap_.clear();
}

void
Hash::
printFilePathStats()
{
  if (filePathActive_)
    std::cout << filePathMap_.size() << " entries.\n";
  else
    std::cout << "inactive.\n";
}

void
Hash::
setFilePathActive(bool flag)
{
  filePathActive_ = flag;
}

}
