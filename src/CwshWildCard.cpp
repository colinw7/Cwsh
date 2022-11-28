#include <CwshI.h>
#include <CGlob.h>

namespace Cwsh {

WildCard::
WildCard(const std::string &pattern)
{
  glob_ = std::make_shared<CGlob>(pattern);

  glob_->setAllowOr(false);
  glob_->setAllowNonPrintable(true);
}

WildCard::
~WildCard()
{
}

bool
WildCard::
isValid() const
{
  return glob_->isPattern();
}

bool
WildCard::
checkMatch(const std::string &str) const
{
  return glob_->compare(str);
}

}
