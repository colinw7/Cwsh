#ifndef CMESSAGE_H
#define CMESSAGE_H

#include <map>
#include <string>
#include <cstddef>
#include <sys/types.h>

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

#define CMessageMgrInst CMessageMgr::getInstance()

//---

class CMessage {
 public:
  static bool isActive(const std::string &id);

  CMessage(const std::string &id);
 ~CMessage();

  bool sendClientMessage(const std::string &msg);
  bool sendClientData(int type, const char *data, int len);

  bool recvClientMessage(std::string &msg);
  bool recvClientData(int &type, char* &data, int &len);

  bool sendServerMessage(const std::string &msg, int errorCode=0);
  bool sendServerData(int type, const char *data, int len, int errorCode=0);

  bool recvServerMessage(std::string &msg, int *errorCode);
  bool recvServerData(int &type, char* &data, int &len);

  void sendClientPending();

  bool sendClientMessageAndRecv(const std::string &msg, std::string &reply);

 private:
  int createSharedMem();

  void initSharedMem();

  int  getShmId();
  void setShmId(int integer);

  void setShmNum(int integer);
  void incShmNum();
  bool decShmNum();

  static int getShmId(const std::string &idFilename);

  //---

  std::size_t messageSize() const { return sizeof(MessageData) + maxData_; }

  std::size_t memorySize() const { return sizeof(MessageMemory) + numMessages_*messageSize(); }

 private:
  enum class Type {
    STRING
  };

  // message data (header and payload)
  struct MessageData {
    uint uid; // fixed

    char state[4]; // flags

    uint id;
    uint type;
    uint len;
    int  errorCode;
    char data[4];

    bool isPending() const { return state[0]; }
    void setPending(bool b) { state[0] = b; }

    bool isClient() const { return state[1]; }
    void setClient(bool b) { state[1] = b; }
  };

  // shared memory (header and messages)
  struct MessageMemory {
    uint uid;
    char data[4];
  };

  struct BufferData {
    uint        num        { 0 };
    uint        pos        { 0 };
    char*       mem        { nullptr };
    std::size_t memSize    { 0 };
    bool        enabled    { true };
  };

  std::string id_;
  int         shmId_        { 0 };
  std::string idFilename_;
  std::string numFilename_;
  bool        debug_        { false };
  uint        lastId_       { 0 };
  uint        numMessages_  { 1024 };
  uint        numErrors_    { 0 };
  uint        maxData_      { 1024 };
  uint        numBuffers_   { 1024 };
  BufferData  bufferData_;
};

#endif
