#ifndef CWSH_STATE_H
#define CWSH_STATE_H

namespace Cwsh {

class StateMgr {
 public:
  StateMgr(App *cwsh);
 ~StateMgr() { }

  void save(App *cwsh);
  void restore();

 private:
  using StateP     = std::shared_ptr<State>;
  using StateArray = std::vector<StateP>;

  CPtr<App>  cwsh_;
  StateP     currentState_;
  StateArray stateStack_;
};

//---

class State {
 public:
  State(App *cwsh);
 ~State();

 private:
  CPtr<App>   cwsh_;
  std::string dir_;
};

}
#endif
