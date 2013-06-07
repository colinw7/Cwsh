#include "CwshI.h"
#include <CGlob.h>

CwshWildCard::
CwshWildCard(const string &pattern)
{
  glob_ = new CGlob(pattern);

  glob_->setAllowOr(false);
  glob_->setAllowNonPrintable(true);
}

CwshWildCard::
~CwshWildCard()
{
}

bool
CwshWildCard::
isValid() const
{
  return glob_->isPattern();
}

bool
CwshWildCard::
checkMatch(const string &str) const
{
  return glob_->compare(str);
}
