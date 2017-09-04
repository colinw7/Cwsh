#include <CwshI.h>

CwshHash::
CwshHash(Cwsh *cwsh) :
 cwsh_(cwsh)
{
}

void
CwshHash::
addFilePath(const std::string &filename, const std::string &path)
{
  if (file_path_active_) {
    file_path_map_[filename] = path;

    if (cwsh_->getDebug())
      std::cerr << "Added (" << filename << "," << path << ") to hash." << std::endl;
  }
}

std::string
CwshHash::
getFilePath(const std::string &filename)
{
  if (file_path_active_) {
    auto p = file_path_map_.find(filename);

    if (p != file_path_map_.end()) {
      if (cwsh_->getDebug())
        std::cerr << "Found (" << filename << "," << (*p).second << ") in hash." << std::endl;

      return (*p).second;
    }

    return "";
  }
  else
    return "";
}

void
CwshHash::
clearFilePath()
{
  file_path_map_.clear();
}

void
CwshHash::
printFilePathStats()
{
  if (file_path_active_)
    std::cout << file_path_map_.size() << " entries." << std::endl;
  else
    std::cout << "inactive." << std::endl;
}

void
CwshHash::
setFilePathActive(bool flag)
{
  file_path_active_ = flag;
}
