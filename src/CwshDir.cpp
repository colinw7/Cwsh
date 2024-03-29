#include <CwshI.h>

namespace Cwsh {

std::string
Dir::
lookup(App *cwsh, const std::string &dirname, bool required)
{
  if (CFile::exists(dirname) && CFile::isDirectory(dirname))
    return dirname;

  auto len = dirname.size();

  if (len > 0 && (dirname[0] == '/' || dirname[0] == '.')) {
    if (required)
      CWSH_THROWQ(dirname, "No such file or directory.");
    else
      return "";
  }

  auto *variable = cwsh->lookupVariable("cdpath");

  if (! variable) {
    if (required)
      CWSH_THROWQ(dirname, "No such file or directory.");
    else
      return "";
  }

  auto num_values = variable->getNumValues();

  for (uint i = 0; i < num_values; ++i) {
    std::string dirname1 = variable->getValue(i) + "/" + dirname;

    if (CFile::exists(dirname1) && CFile::isDirectory(dirname1)) {
      std::cout << String::replaceHome(dirname1) << "\n";
      return dirname1;
    }
  }

  if (required)
    CWSH_THROWQ(dirname, "No such file or directory.");
  else
    return "";
}

}
