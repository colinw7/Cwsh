#ifndef CWSH_UNIX_COMMAND_H
#define CWSH_UNIX_COMMAND_H

class CwshUnixCommand {
 public:
  static std::string search(Cwsh *cwsh, const std::string &name);
};

#endif
