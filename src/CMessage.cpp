#include <CMessage.h>
#include <CFile.h>
#include <CStrUtil.h>
#include <COSTimer.h>

#include <iostream>
#include <cstring>
#include <sys/ipc.h>
#include <sys/shm.h>

class CMessageLock {
 public:
  CMessageLock(int id);
 ~CMessageLock();

  void *getData() const { return data_; }

 private:
  void *data_ { nullptr };
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
  std::string id_filename = CMessageMgrInst->getIdFilename(id);

  int shm_id = getShmId(id_filename);

  return (shm_id != 0);
}

CMessage::
CMessage(const std::string &id) :
 id_(id), debug_(false)
{
  if (getenv("CMESSAGE_DEBUG") != NULL)
    debug_ = true;

  id_filename_  = CMessageMgrInst->getIdFilename (id_);
  num_filename_ = CMessageMgrInst->getNumFilename(id_);

  shm_id_ = getShmId(id_filename_);

  if (shm_id_ == 0) {
    shm_id_ = createSharedMem();

    setShmId (shm_id_);
    setShmNum(1);
  }
  else
    incShmNum();
}

CMessage::
~CMessage()
{
  if (decShmNum()) {
    if (debug_)
      std::cerr << "remove shm id " << shm_id_ << std::endl;

    shmctl(shm_id_, IPC_RMID, NULL);

    if (debug_)
      std::cerr << "remove files " << id_filename_ << " " << num_filename_ << std::endl;

    CFile::remove(id_filename_);
    CFile::remove(num_filename_);
  }
}

int
CMessage::
createSharedMem()
{
  int shm_id =
    shmget(IPC_PRIVATE, sizeof(MessageData), IPC_CREAT | IPC_EXCL | 0600);

  if (shm_id == -1)
    perror("shmget");

  return shm_id;
}

bool
CMessage::
sendClientMessage(const std::string &msg)
{
  CMessageLock lock(shm_id_);

  MessageData *data = (MessageData *) lock.getData();

  if (data == NULL)
    return false;

  ClientMessageData *client_data = &data->client_data;

  if (client_data->pending)
    return false;

  client_data->len = msg.size();

  if (client_data->len >= (int) sizeof(client_data->msg))
    return false;

  strncpy(client_data->msg, msg.c_str(), client_data->len);
  client_data->msg[client_data->len] = '\0';

  client_data->pending = true;

  return true;
}

bool
CMessage::
recvClientMessage(std::string &msg)
{
  CMessageLock lock(shm_id_);

  MessageData *data = (MessageData *) lock.getData();

  if (data == NULL)
    return false;

  ClientMessageData *client_data = &data->client_data;

  if (! client_data->pending)
    return false;

  msg = std::string(client_data->msg, client_data->len);

  client_data->pending = false;

  return true;
}

bool
CMessage::
sendServerMessage(const std::string &msg, int error_code)
{
  CMessageLock lock(shm_id_);

  MessageData *data = (MessageData *) lock.getData();

  if (data == NULL)
    return false;

  ServerMessageData *server_data = &data->server_data;

  if (server_data->pending)
    return false;

  server_data->len = msg.size();

  if (server_data->len >= (int) sizeof(server_data->msg))
    return false;

  strncpy(server_data->msg, msg.c_str(), server_data->len);
  server_data->msg[server_data->len] = '\0';

  server_data->error_code = error_code;

  server_data->pending = true;

  return true;
}

bool
CMessage::
recvServerMessage(std::string &msg, int *error_code)
{
  CMessageLock lock(shm_id_);

  MessageData *data = (MessageData *) lock.getData();

  if (data == NULL)
    return false;

  ServerMessageData *server_data = &data->server_data;

  if (! server_data->pending)
    return false;

  msg = std::string(server_data->msg, server_data->len);

  *error_code = server_data->error_code;

  server_data->pending = false;

  return true;
}

bool
CMessage::
sendClientMessageAndRecv(const std::string &msg, std::string &reply)
{
  reply = "";

  sendClientMessage(msg);

  for (int i = 0; i < 10000; ++i) {
    COSTimer::msleep(50);

    int error_code;

    if (recvServerMessage(reply, &error_code))
      return true;
  }

  return false;
}

int
CMessage::
getShmId()
{
  return getShmId(id_filename_);
}

int
CMessage::
getShmId(const std::string &id_filename)
{
  int integer = 0;

  if (CFile::exists(id_filename)) {
    CFile file(id_filename);

    std::string line;

    file.readLine(line);

    CStrUtil::toInteger(line, &integer);
  }

  CMessageLock lock(integer);

  MessageData *data = (MessageData *) lock.getData();

  if (data == NULL)
    return 0;

  return integer;
}

void
CMessage::
setShmId(int integer)
{
  CFile file(id_filename_);

  std::string line = CStrUtil::toString(integer) + "\n";

  file.write(line);

  if (debug_)
    std::cerr << "setShmId " << integer << std::endl;
}

void
CMessage::
setShmNum(int integer)
{
  CMessageLock lock(shm_id_);

  CFile file(num_filename_);

  std::string line = CStrUtil::toString(integer) + "\n";

  file.write(line);

  if (debug_)
    std::cerr << "setShmNum " << integer << std::endl;
}

void
CMessage::
incShmNum()
{
  CMessageLock lock(shm_id_);

  int integer = 0;

  if (CFile::exists(num_filename_)) {
    CFile file(num_filename_);

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

    CFile file(num_filename_);

    std::string line = CStrUtil::toString(integer) + "\n";

    file.write(line);
  }

  if (debug_)
    std::cerr << "incShmNum " << integer << std::endl;
}

bool
CMessage::
decShmNum()
{
  CMessageLock lock(shm_id_);

  int integer = 0;

  if (CFile::exists(num_filename_)) {
    CFile file(num_filename_);

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

    CFile file(num_filename_);

    std::string line = CStrUtil::toString(integer) + "\n";

    file.write(line);
  }

  if (debug_)
    std::cerr << "decShmNum " << integer << std::endl;

  return (integer == 0);
}

//--------

CMessageLock::
CMessageLock(int id)
{
  data_ = shmat(id, NULL, SHM_RND);

  if (data_ == (void *) -1)
    data_ = NULL;
}

CMessageLock::
~CMessageLock()
{
  shmdt(data_);
}
