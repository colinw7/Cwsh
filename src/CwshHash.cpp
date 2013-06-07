#include "CwshI.h"

CwshHash::
CwshHash(Cwsh *cwsh) :
 cwsh_(cwsh), file_path_active_(false)
{
}

void
CwshHash::
addFilePath(const string &filename, const string &path)
{
  if (file_path_active_) {
    file_path_map_[filename] = path;

    if (cwsh_->getDebug())
      cerr << "Added (" << filename << "," << path << ") to hash." << endl;
  }
}

string
CwshHash::
getFilePath(const string &filename)
{
  if (file_path_active_) {
    CwshHashFilePathMap::iterator p = file_path_map_.find(filename);

    if (p != file_path_map_.end()) {
      if (cwsh_->getDebug())
        cerr << "Found (" << filename << "," << (*p).second << ") in hash." << endl;

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
    cout << file_path_map_.size() << " entries." << endl;
  else
    cout << "inactive." << endl;
}

void
CwshHash::
setFilePathActive(bool flag)
{
  file_path_active_ = flag;
}
