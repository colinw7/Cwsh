#include <CwshI.h>

std::string
CwshDir::
lookup(Cwsh *cwsh, const std::string &dirname, bool required)
{
  if (CFile::exists(dirname) && CFile::isDirectory(dirname))
    return dirname;

  int len = dirname.size();

  if (len > 0 && (dirname[0] == '/' || dirname[0] == '.')) {
    if (required)
      CWSH_THROWQ(dirname, "No such file or directory.");
    else
      return "";
  }

  CwshVariable *variable = cwsh->lookupVariable("cdpath");

  if (! variable) {
    if (required)
      CWSH_THROWQ(dirname, "No such file or directory.");
    else
      return "";
  }

  int num_values = variable->getNumValues();

  for (int i = 0; i < num_values; ++i) {
    std::string dirname1 = variable->getValue(i) + "/" + dirname;

    if (CFile::exists(dirname1) && CFile::isDirectory(dirname1)) {
      std::cout << CwshString::replaceHome(dirname1) << std::endl;
      return dirname1;
    }
  }

  if (required)
    CWSH_THROWQ(dirname, "No such file or directory.");
  else
    return "";
}
