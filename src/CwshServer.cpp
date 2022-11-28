#include <CwshI.h>
#include <CMessage.h>

namespace Cwsh {

Server::
Server(App *cwsh) :
 cwsh_(cwsh)
{
  message_ = createMessage();
}

Server::
~Server()
{
}

Server::MessageP
Server::
createMessage()
{
  return std::make_shared<CMessage>("CwshServer");
}

bool
Server::
processMessage()
{
  std::string msg;

  if (! message_->recvClientMessage(msg))
    return false;

  uint len = uint(msg.size());

  std::string reply;

  if      (msg == "get_aliases")
    reply = cwsh_->getAliasesMsg();
  else if (len > 10 && msg.substr(0, 10) == "set_alias ") {
    std::string nameValue = CStrUtil::stripSpaces(msg.substr(10));

    std::string::size_type pos = nameValue.find(' ');

    if (pos != std::string::npos) {
      std::string name  = nameValue.substr(0, pos);
      std::string value = CStrUtil::stripSpaces(nameValue.substr(pos));

      cwsh_->defineAlias(name, value);

      reply = "1";
    }
    else
      reply = "1";
  }
  else if (len > 12 && msg.substr(0, 12) == "unset_alias ") {
    std::string name = CStrUtil::stripSpaces(msg.substr(12));

    cwsh_->undefineAlias(name);

    reply = "1";
  }
  else if (msg == "get_history")
    reply = cwsh_->getHistoryMsg();
  else if (len > 3 && msg.substr(0, 3) == "cd ") {
    std::string dirname = CStrUtil::stripSpaces(msg.substr(3));

    cwsh_->changeDir(dirname);

    reply = dirname;
  }
  else if (msg == "pwd")
    reply = COSFile::getCurrentDir();
  else
    reply = "<Unknown Command> " + msg;

  message_->sendServerMessage(reply, 0);

  return true;
}

}
