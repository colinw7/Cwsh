#ifndef CWSH_SERVER_H
#define CWSH_SERVER_H

class CMessage;

class CwshServer {
 public:
  CwshServer(Cwsh *cwsh);
 ~CwshServer();

  bool processMessage();

  static CMessage *createMessage();

 private:
  CPtr<Cwsh>         cwsh_;
  CAutoPtr<CMessage> message_;
};

#endif
