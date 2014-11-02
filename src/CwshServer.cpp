#include <CwshI.h>

#ifdef USE_MESSAGES
#include <CMessage.h>
#endif

CwshServer::
CwshServer(Cwsh *cwsh) :
 cwsh_(cwsh)
{
#ifdef USE_MESSAGES
  message_ = createMessage();
#endif
}

CwshServer::
~CwshServer()
{
}

CMessage *
CwshServer::
createMessage()
{
#ifdef USE_MESSAGES
  return new CMessage("CwshServer");
#else
  return 0;
#endif
}

bool
CwshServer::
processMessage()
{
#ifdef USE_MESSAGES
  string msg;

  if (! message_->recvClientMessage(msg))
    return false;

  uint len = msg.size();

  string reply;

  if      (msg == "get_aliases")
    reply = cwsh_->getAliasesMsg();
  else if (len > 10 && msg.substr(0, 10) == "set_alias ") {
    string nameValue = CStrUtil::stripSpaces(msg.substr(10));

    string::size_type pos = nameValue.find(' ');

    if (pos != string::npos) {
      string name  = nameValue.substr(0, pos);
      string value = CStrUtil::stripSpaces(nameValue.substr(pos));

      cwsh_->defineAlias(name, value);

      reply = "1";
    }
    else
      reply = "1";
  }
  else if (len > 12 && msg.substr(0, 12) == "unset_alias ") {
    string name = CStrUtil::stripSpaces(msg.substr(12));

    cwsh_->undefineAlias(name);

    reply = "1";
  }
  else if (msg == "get_history")
    reply = cwsh_->getHistoryMsg();
  else if (len > 3 && msg.substr(0, 3) == "cd ") {
    string dirname = CStrUtil::stripSpaces(msg.substr(3));

    cwsh_->changeDir(dirname);

    reply = dirname;
  }
  else if (msg == "pwd")
    reply = COSFile::getCurrentDir();
  else
    reply = "<Unknown Command> " + msg;

  message_->sendServerMessage(reply, 0);

  return true;
#else
  return false;
#endif
}
