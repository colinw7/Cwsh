#include <CwshI.h>
#include <CPathList.h>

namespace Cwsh {

std::string
UnixCommand::
search(App *cwsh, const std::string &name)
{
  CPathList pathList;

  pathList.addEnvValue("PATH");

  std::string path;

  if (! pathList.search(name, path))
    CWSH_THROW(name + ": Command not found.");

  cwsh->addFilePath(name, path);

  return path;
}

}
