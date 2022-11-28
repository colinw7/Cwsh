#include <CwshI.h>

namespace Cwsh {

StateMgr::
StateMgr(App *cwsh) :
 cwsh_(cwsh)
{
}

void
StateMgr::
save(App *cwsh)
{
  if (currentState_)
    stateStack_.push_back(currentState_);

  currentState_ = std::make_shared<State>(cwsh);
}

void
StateMgr::
restore()
{
  if (! currentState_)
    CWSH_THROW("Not in saved state.");

  if (! stateStack_.empty()) {
    auto state = stateStack_.back();

    stateStack_.pop_back();

    currentState_ = state;
  }
  else
    currentState_ = StateP();
}

//-----

State::
State(App *cwsh) :
 cwsh_(cwsh)
{
  dir_ = COSFile::getCurrentDir();

  cwsh_->saveVariables();
}

State::
~State()
{
  cwsh_->changeDir(dir_);

  cwsh_->restoreVariables();
}

}
