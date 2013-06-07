#include "CwshI.h"
#include "CPathList.h"

string
CwshUnixCommand::
search(Cwsh *cwsh, const string &name)
{
  CPathList pathList;

  pathList.addEnvValue("PATH");

  string path;

  if (! pathList.search(name, path))
    CWSH_THROW(name + ": Command not found.");

  cwsh->addFilePath(name, path);

  return path;
}
