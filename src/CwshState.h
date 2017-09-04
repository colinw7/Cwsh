#ifndef CWSH_STATE_H
#define CWSH_STATE_H

class CwshStateMgr {
 public:
  CwshStateMgr(Cwsh *cwsh);
 ~CwshStateMgr() { }

  void save(Cwsh *cwsh);
  void restore();

 private:
  typedef CAutoPtrStack<CwshState> StateArray;

  CPtr<Cwsh>          cwsh_;
  CAutoPtr<CwshState> current_state_;
  StateArray          state_stack_;
};

//---

class CwshState {
 public:
  CwshState(Cwsh *cwsh);
 ~CwshState();

 private:
  CPtr<Cwsh>  cwsh_;
  std::string dir_;
};

#endif
