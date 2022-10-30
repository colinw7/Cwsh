#ifndef CBash_H
#define CBash_H

#include <string>
#include <vector>
#include <map>
#include <functional>

class CReadLine;

namespace CBash {

using StringArray = std::vector<std::string>;

struct Cmd {
  std::string name;
  StringArray args;
};

class App;

class BuiltinCommand {
 public:
  BuiltinCommand(App *app) :
   app_(app) {
  }

  virtual ~BuiltinCommand() { }

  virtual void exec(const Cmd &cmd) = 0;

 protected:
  App *app_ { nullptr };
};

class App {
 public:
  App();
 ~App();

  void init(int argc, char **argv);

  void addShellCommands();

  void mainLoop();

  bool eof() const;

  std::string getLine() const;

  void parseCommand(const std::string &line, Cmd &cmd) const;

  void expandArg(const std::string &arg, std::vector<std::string> &args) const;

  void processCommand(const Cmd &cmd);

  std::string lookupDir(const std::string &dirname) const;

  bool changeDir(const std::string &dirname) const;

 private:
  using Commands = std::map<std::string, BuiltinCommand *>;

  CReadLine *readLine_ { nullptr };
  Commands   shellCommands_;
};

class CdCommand : public BuiltinCommand {
 public:
  CdCommand(App *app) :
   BuiltinCommand(app) {
  }

  void exec(const Cmd &cmd) override;
};

class ExitCommand : public BuiltinCommand {
 public:
  ExitCommand(App *app) :
   BuiltinCommand(app) {
  }

  void exec(const Cmd &cmd) override;
};

}

#endif
