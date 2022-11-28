#ifndef CWSH_SERVER_H
#define CWSH_SERVER_H

class CMessage;

namespace Cwsh {

class Server {
 public:
  using MessageP = std::shared_ptr<CMessage>;

 public:
  Server(App *cwsh);
 ~Server();

  bool processMessage();

  static MessageP createMessage();

 private:
  CPtr<App> cwsh_;
  MessageP  message_;
};

}

#endif
