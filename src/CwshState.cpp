#include <CwshI.h>

CwshStateMgr::
CwshStateMgr(Cwsh *cwsh) :
 cwsh_(cwsh)
{
}

void
CwshStateMgr::
save(Cwsh *cwsh)
{
  if (current_state_) {
    state_stack_.push(current_state_);

    current_state_.release();
  }

  current_state_ = new CwshState(cwsh);
}

void
CwshStateMgr::
restore()
{
  if (! current_state_)
    CWSH_THROW("Not in saved state.");

  if (! state_stack_.empty()) {
    CwshState *state;

    state_stack_.pop(&state);

    current_state_ = state;
  }
  else
    current_state_ = nullptr;
}

//-----

CwshState::
CwshState(Cwsh *cwsh) :
 cwsh_(cwsh)
{
  dir_ = COSFile::getCurrentDir();

  cwsh_->saveVariables();
}

CwshState::
~CwshState()
{
  cwsh_->changeDir(dir_);

  cwsh_->restoreVariables();
}
