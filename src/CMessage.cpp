#include <CMessage.h>
#include <CFile.h>
#include <CStrUtil.h>
#include <COSTimer.h>

#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <sys/ipc.h>
#include <sys/shm.h>

class CMessageLock {
 public:
  CMessageLock(int id);
 ~CMessageLock();

  void *getData() const { return data_; }

 private:
  static bool locked_;

  void* data_ { nullptr };
};

//---------

CMessageMgr *
CMessageMgr::
getInstance()
{
  static CMessageMgr *instance;

  if (! instance)
    instance = new CMessageMgr;

  return instance;
}

CMessageMgr::
CMessageMgr()
{
}

CMessageMgr::
~CMessageMgr()
{
}

std::string
CMessageMgr::
getIdFilename(const std::string &id)
{
  std::string id1 = CStrUtil::toUpper(CStrUtil::stripSpaces(id));

  return "/tmp/" + id1 + "_SHM_ID";
}

std::string
CMessageMgr::
getNumFilename(const std::string &id)
{
  std::string id1 = CStrUtil::toUpper(CStrUtil::stripSpaces(id));

  return "/tmp/" + id1 + "_SHM_NUM";
}

//---------

bool
CMessage::
isActive(const std::string &id)
{
  std::string idFilename = CMessageMgrInst->getIdFilename(id);

  int shmId = getShmId(idFilename);

  return (shmId != 0);
}

CMessage::
CMessage(const std::string &id) :
 id_(id), debug_(false)
{
  if (getenv("CMESSAGE_DEBUG"))
    debug_ = true;

  idFilename_  = CMessageMgrInst->getIdFilename (id_);
  numFilename_ = CMessageMgrInst->getNumFilename(id_);

  shmId_ = getShmId(idFilename_);

  if (shmId_ == 0) {
    shmId_ = createSharedMem();

    setShmId (shmId_);
    setShmNum(1);

    initSharedMem();
  }
  else
    incShmNum();

  std::size_t messageSize = this->messageSize();

  bufferData_.memSize = numBuffers_*messageSize;

  bufferData_.mem = new char [bufferData_.memSize];
}

CMessage::
~CMessage()
{
  if (decShmNum()) {
    if (debug_)
      std::cerr << "remove shm id " << shmId_ << "\n";

    shmctl(shmId_, IPC_RMID, nullptr);

    if (debug_)
      std::cerr << "remove files " << idFilename_ << " " << numFilename_ << "\n";

    CFile::remove(idFilename_);
    CFile::remove(numFilename_);
  }

  delete [] bufferData_.mem;
}

int
CMessage::
createSharedMem()
{
  int shmId = shmget(IPC_PRIVATE, memorySize(), IPC_CREAT | IPC_EXCL | 0600);

  if (shmId == -1)
    perror("shmget");

  return shmId;
}

void
CMessage::
initSharedMem()
{
  CMessageLock lock(shmId_);

  auto *messageMemory = (MessageMemory *) lock.getData();
  assert(messageMemory);

  memset(messageMemory, 0, memorySize());

  messageMemory->uid = 0xBEADFEED;

  auto *messageAddr = (char *) &messageMemory->data[0];

  std::size_t messageSize = this->messageSize();

  for (uint i = 0; i < numMessages_; ++i) {
    auto *messageData = (MessageData *) messageAddr;

    messageData->uid = 0xFEEDBEAD;

    messageAddr += messageSize;
  }
}

//---

bool
CMessage::
sendClientMessage(const std::string &msg)
{
  return sendClientData((int) Type::STRING, msg.c_str(), msg.size() + 1);
}

bool
CMessage::
sendClientData(int type, const char *data, int len)
{
  if (len >= (int) maxData_) {
    ++numErrors_;
    std::cerr << "CMessage::sendClientData : message too large (" << numErrors_ << ")\n";
    return false;
  }

  //---

  CMessageLock lock(shmId_);

  auto *messageMemory = (MessageMemory *) lock.getData();

  if (! messageMemory) {
    ++numErrors_;
    std::cerr << "CMessage::sendClientData : lock failed (" << numErrors_ << ")\n";
    return false;
  }

  assert(messageMemory->uid == 0xBEADFEED);

  //---

  // find slot for message
  std::size_t messageSize = this->messageSize();

  uint  messageNum  = 0;
  auto *messageAddr = (char *) &messageMemory->data[0];

  while (messageNum < numMessages_) {
    auto *messageData = (MessageData *) messageAddr;

    assert(messageData->uid == 0xFEEDBEAD);

    if (! messageData->isPending())
      break;

    ++messageNum;

    messageAddr += messageSize;
  }

  //---

  // use slot if found
  if (messageNum < numMessages_) {
    auto *messageData = (MessageData *) messageAddr;

    messageData->setPending(true);
    messageData->setClient (true);

    messageData->id        = ++lastId_;
    messageData->type      = type;
    messageData->len       = len;
    messageData->errorCode = 0;

    char *clientData = &messageData->data[0];

    memcpy(clientData, data, len);

    //---

    numErrors_ = 0;

    return true;
  }

  //--

  // store in buffer if memory full
  if (bufferData_.enabled) {
    if (bufferData_.num < numBuffers_) {
      uint pos = bufferData_.pos + bufferData_.num;

      if (pos >= numBuffers_)
        pos -= numBuffers_;

      int messagePos = messageSize*pos;

      assert(messagePos + messageSize <= bufferData_.memSize);

      auto *bufferMessageData = (MessageData *) &bufferData_.mem[messagePos];

      bufferMessageData->setPending(true);
      bufferMessageData->setClient (true);

      bufferMessageData->id        = ++lastId_;
      bufferMessageData->type      = type;
      bufferMessageData->len       = len;
      bufferMessageData->errorCode = 0;

      char *bufferData = &bufferMessageData->data[0];

      memcpy(bufferData, data, len);

      ++bufferData_.num;

      std::cerr << "CMessage::sendClientData : message buffered (" << bufferData_.num << ")\n";

      return true;
    }
  }

  //---

  ++numErrors_;

  std::cerr << "CMessage::sendClientData : message pending (" << numErrors_ << ")\n";

  return false;
}

bool
CMessage::
recvClientMessage(std::string &msg)
{
  CMessageLock lock(shmId_);

  auto *messageMemory = (MessageMemory *) lock.getData();
  if (! messageMemory) return false;

  assert(messageMemory->uid == 0xBEADFEED);

  //---

  // find next pending string message
  std::size_t messageSize = this->messageSize();

  uint  messageNum  = 0;
  auto *messageAddr = (char *) &messageMemory->data[0];

  while (messageNum < numMessages_) {
    auto *messageData = (MessageData *) messageAddr;

    assert(messageData->uid == 0xFEEDBEAD);

    if (messageData->isPending() && messageData->isClient() &&
        messageData->type == (int) Type::STRING)
      break;

    ++messageNum;

    messageAddr += messageSize;
  }

  //---

  // return message if found
  if (messageNum >= numMessages_)
    return false;

  auto *messageData = (MessageData *) messageAddr;

  messageData->setPending(false);

  char *clientData = &messageData->data[0];

  msg = std::string(clientData, messageData->len);

  return true;
}

bool
CMessage::
recvClientData(int &type, char* &data, int &len)
{
  CMessageLock lock(shmId_);

  auto *messageMemory = (MessageMemory *) lock.getData();
  if (! messageMemory) return false;

  assert(messageMemory->uid == 0xBEADFEED);

  //---

  // find next pending message (any type)
  std::size_t messageSize = this->messageSize();

  uint  messageNum  = 0;
  auto *messageAddr = (char *) &messageMemory->data[0];

  while (messageNum < numMessages_) {
    auto *messageData = (MessageData *) messageAddr;

    assert(messageData->uid == 0xFEEDBEAD);

    if (messageData->isPending() && messageData->isClient())
      break;

    ++messageNum;

    messageAddr += messageSize;
  }

  //---

  // return message if found
  if (messageNum >= numMessages_)
    return false;

  auto *messageData = (MessageData *) messageAddr;

  messageData->setPending(false);

  type = messageData->len;
  len  = messageData->len;

  data = new char [len];

  memcpy(data, &messageData->data[0], len);

  return true;
}

void
CMessage::
sendClientPending()
{
  if (bufferData_.num > 0) {
    std::size_t messageSize = this->messageSize();

    uint pos = bufferData_.pos + bufferData_.num;

    if (pos >= numBuffers_)
      pos -= numBuffers_;

    int messagePos = messageSize*pos;

    assert(messagePos + messageSize <= bufferData_.memSize);

    auto *bufferMessageData = (MessageData *) &bufferData_.mem[messagePos];

    assert(bufferMessageData->isPending());

    if (! bufferMessageData->isClient())
      return;

    char *bufferData = &bufferMessageData->data[0];

    bufferData_.enabled = false;

    if (sendClientData(bufferMessageData->type, bufferData, bufferMessageData->len)) {
      ++bufferData_.pos;
      --bufferData_.num;

      if (bufferData_.pos >= numBuffers_)
        bufferData_.pos -= numBuffers_;
    }

    bufferData_.enabled = true;
  }
}

//---

bool
CMessage::
sendServerMessage(const std::string &msg, int errorCode)
{
  return sendServerData((int) Type::STRING, msg.c_str(), msg.size() + 1, errorCode);
}

bool
CMessage::
sendServerData(int type, const char *data, int len, int errorCode)
{
  if (len >= (int) maxData_) {
    ++numErrors_;
    std::cerr << "CMessage::sendServerData : message too large (" << numErrors_ << ")\n";
    return false;
  }

  //---

  CMessageLock lock(shmId_);

  auto *messageMemory = (MessageMemory *) lock.getData();

  if (! messageMemory) {
    ++numErrors_;
    std::cerr << "CMessage::sendServerData : lock failed (" << numErrors_ << ")\n";
    return false;
  }

  assert(messageMemory->uid == 0xBEADFEED);

  //---

  // find slot for message
  std::size_t messageSize = this->messageSize();

  uint  messageNum  = 0;
  auto *messageAddr = (char *) &messageMemory->data[0];

  while (messageNum < numMessages_) {
    auto *messageData = (MessageData *) messageAddr;

    assert(messageData->uid == 0xFEEDBEAD);

    if (! messageData->isPending())
      break;

    ++messageNum;

    messageAddr += messageSize;
  }

  //---

  // use slot if found
  if (messageNum < numMessages_) {
    auto *messageData = (MessageData *) messageAddr;

    messageData->setPending(true);
    messageData->setClient (false);

    messageData->id        = ++lastId_;
    messageData->type      = type;
    messageData->len       = len;
    messageData->errorCode = errorCode;

    char *serverData = &messageData->data[0];

    memcpy(serverData, data, len);

    //---

    numErrors_ = 0;

    return true;
  }

  //--

  // store in buffer if memory full
  if (bufferData_.enabled) {
    if (bufferData_.num < numBuffers_) {
      uint pos = bufferData_.pos + bufferData_.num;

      if (pos >= numBuffers_)
        pos -= numBuffers_;

      int messagePos = messageSize*pos;

      assert(messagePos + messageSize <= bufferData_.memSize);

      auto *bufferMessageData = (MessageData *) &bufferData_.mem[messagePos];

      bufferMessageData->setPending(true);
      bufferMessageData->setClient (false);

      bufferMessageData->id        = ++lastId_;
      bufferMessageData->type      = type;
      bufferMessageData->len       = len;
      bufferMessageData->errorCode = errorCode;

      char *bufferServerData = &bufferMessageData->data[0];

      memcpy(bufferServerData, data, len);

      ++bufferData_.num;

      std::cerr << "CMessage::sendServerData : message buffered (" << bufferData_.num << ")\n";

      return true;
    }
  }

  ++numErrors_;

  std::cerr << "CMessage::sendServerData : message pending (" << numErrors_ << ")\n";

  return false;
}

bool
CMessage::
recvServerMessage(std::string &msg, int *errorCode)
{
  CMessageLock lock(shmId_);

  auto *messageMemory = (MessageMemory *) lock.getData();
  if (! messageMemory) return false;

  assert(messageMemory->uid == 0xBEADFEED);

  //---

  // find next pending string message
  std::size_t messageSize = this->messageSize();

  uint  messageNum  = 0;
  auto *messageAddr = (char *) &messageMemory->data[0];

  while (messageNum < numMessages_) {
    auto *messageData = (MessageData *) messageAddr;

    assert(messageData->uid == 0xFEEDBEAD);

    if (messageData->isPending() && ! messageData->isClient() &&
        messageData->type == (int) Type::STRING)
      break;

    ++messageNum;

    messageAddr += messageSize;
  }

  //---

  // return message if found
  if (messageNum >= numMessages_)
    return false;

  auto *messageData = (MessageData *) messageAddr;

  messageData->setPending(false);

  char *serverData = &messageData->data[0];

  msg = std::string(serverData, messageData->len);

  *errorCode = messageData->errorCode;

  return true;
}

bool
CMessage::
recvServerData(int &type, char* &data, int &len)
{
  CMessageLock lock(shmId_);

  auto *messageMemory = (MessageMemory *) lock.getData();
  if (! messageMemory) return false;

  assert(messageMemory->uid == 0xBEADFEED);

  //---

  // find next pending message (any type)
  std::size_t messageSize = this->messageSize();

  uint  messageNum  = 0;
  auto *messageAddr = (char *) &messageMemory->data[0];

  while (messageNum < numMessages_) {
    auto *messageData = (MessageData *) messageAddr;

    assert(messageData->uid == 0xFEEDBEAD);

    if (messageData->isPending() && ! messageData->isClient())
      break;

    ++messageNum;

    messageAddr += messageSize;
  }

  //---

  // return message if found
  if (messageNum >= numMessages_)
    return false;

  auto *messageData = (MessageData *) messageAddr;

  messageData->setPending(false);

  type = messageData->len;
  len  = messageData->len;

  data = new char [len];

  memcpy(data, &messageData->data[0], len);

  return true;
}

//---

bool
CMessage::
sendClientMessageAndRecv(const std::string &msg, std::string &reply)
{
  reply = "";

  sendClientMessage(msg);

  for (int i = 0; i < 10000; ++i) {
    COSTimer::msleep(50);

    int errorCode;

    if (recvServerMessage(reply, &errorCode))
      return true;
  }

  return false;
}

//---

int
CMessage::
getShmId()
{
  return getShmId(idFilename_);
}

int
CMessage::
getShmId(const std::string &idFilename)
{
  int integer = 0;

  if (CFile::exists(idFilename)) {
    CFile file(idFilename);

    std::string line;

    file.readLine(line);

    CStrUtil::toInteger(line, &integer);
  }

  CMessageLock lock(integer);

  auto *messageMemory = (MessageMemory *) lock.getData();
  if (! messageMemory) return 0;

  return integer;
}

void
CMessage::
setShmId(int integer)
{
  CFile file(idFilename_);

  std::string line = CStrUtil::toString(integer) + "\n";

  file.write(line);

  if (debug_)
    std::cerr << "setShmId " << integer << "\n";
}

void
CMessage::
setShmNum(int integer)
{
  CMessageLock lock(shmId_);

  CFile file(numFilename_);

  std::string line = CStrUtil::toString(integer) + "\n";

  file.write(line);

  if (debug_)
    std::cerr << "setShmNum " << integer << "\n";
}

void
CMessage::
incShmNum()
{
  CMessageLock lock(shmId_);

  int integer = 0;

  if (CFile::exists(numFilename_)) {
    CFile file(numFilename_);

    std::string line;

    file.readLine(line);

    CStrUtil::toInteger(line, &integer);

    ++integer;

    file.rewind();

    line = CStrUtil::toString(integer) + "\n";

    file.write(line);
  }
  else {
    integer = 1;

    CFile file(numFilename_);

    std::string line = CStrUtil::toString(integer) + "\n";

    file.write(line);
  }

  if (debug_)
    std::cerr << "incShmNum " << integer << "\n";
}

bool
CMessage::
decShmNum()
{
  CMessageLock lock(shmId_);

  int integer = 0;

  if (CFile::exists(numFilename_)) {
    CFile file(numFilename_);

    std::string line;

    file.readLine(line);

    CStrUtil::toInteger(line, &integer);

    --integer;

    file.rewind();

    line = CStrUtil::toString(integer) + "\n";

    file.write(line);
  }
  else {
    integer = 0;

    CFile file(numFilename_);

    std::string line = CStrUtil::toString(integer) + "\n";

    file.write(line);
  }

  if (debug_)
    std::cerr << "decShmNum " << integer << "\n";

  return (integer == 0);
}

//--------

bool CMessageLock::locked_ = false;

CMessageLock::
CMessageLock(int id)
{
  assert(! locked_);

  locked_ = true;

  data_ = shmat(id, nullptr, SHM_RND);

  if (data_ == (void *) -1)
    data_ = nullptr;
}

CMessageLock::
~CMessageLock()
{
  assert(locked_);

  locked_ = false;

  shmdt(data_);
}
