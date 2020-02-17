#ifndef CMESSAGE_H
#define CMESSAGE_H

#include <map>

#define CMessageMgrInst CMessageMgr::getInstance()

class CMessageMgr {
 public:
  static CMessageMgr *getInstance();

  std::string getIdFilename (const std::string &id);
  std::string getNumFilename(const std::string &id);

 private:
  CMessageMgr();
 ~CMessageMgr();

 private:
  typedef std::map<std::string,int> IdMap;

  IdMap idMap_;
};

class CMessage {
 public:
  static bool isActive(const std::string &id);

  CMessage(const std::string &id);
 ~CMessage();

  bool sendClientMessage(const std::string &msg);
  bool recvClientMessage(std::string &msg);
  bool sendServerMessage(const std::string &msg, int error_code = 0);
  bool recvServerMessage(std::string &msg, int *error_code);

  bool sendClientMessageAndRecv(const std::string &msg, std::string &reply);

 private:
  int createSharedMem();

  int  getShmId();
  void setShmId(int integer);

  void setShmNum(int integer);
  void incShmNum();
  bool decShmNum();

  static int getShmId(const std::string &id_filename);

 private:
  struct ClientMessageData {
    int  pending;
    char msg[32768];
    int  len;
  };

  struct ServerMessageData {
    int  pending;
    char msg[32768];
    int  len;
    int  error_code;
  };

  struct MessageData {
    ClientMessageData client_data;
    ServerMessageData server_data;
  };

  std::string id_;
  int         shm_id_ { 0 };
  std::string id_filename_;
  std::string num_filename_;
  bool        debug_ { false };
};

#endif
