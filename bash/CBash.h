#ifndef CBash_H
#define CBash_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <functional>
#include <memory>
#include <iostream>

class CReadLine;
class CCommand;

namespace CBash {

class App;

using StringArray = std::vector<std::string>;
using StringSet   = std::set<std::string>;

//---

enum class TokenType {
  NONE,
  STRING,
  NUMBER,
  OPERATOR,
  KEYWORD
};

enum class TokenSubType {
  NONE,
  OPEN_RBRACKETS,
  CLOSE_RBRACKETS,
  FILE_INPUT,
  FILE_INPUT_ERR,
  FILE_OUTPUT,
  FILE_OUTPUT_ERR,
  FILE_OUTPUT_PIPE,
  FILE_OUTPUT_APPEND,
  LESS_LESS_MINUS,
  LESS_LESS,
  LESS_GREATER,
  PIPE,
  PIPE_PIPE,
  PIPE_ERR,
  SEMI_COLON,
  SEMI_COLON_BG,
  DOUBLE_SEMI_COLON,
  DOUBLE_SEMI_COLON_BG
};

class Token {
 public:
  Token() { }

  Token(const std::string &str) :
   str_(str) {
  }

  Token(TokenType type, const std::string &str) :
   type_(type), str_(str) {
  }

  const TokenType &type() const { return type_; }

  const TokenSubType &subType() const { return subType_; }
  void setSubType(const TokenSubType &t) { subType_ = t; }

  const std::string &str() const { return str_; }

  std::string sstr() const { return str_.substr(1, str_.size() - 2); }

  void print(std::ostream &os) const {
    switch (type_) {
      case TokenType::STRING  : os << "string"; break;
      case TokenType::NUMBER  : os << "number"; break;
      case TokenType::OPERATOR: os << "op"; break;
      case TokenType::KEYWORD : os << "keyword"; break;
      default                 : os << "none"; break;
    }

    os << "(" << str_ << ")";
    os << "\n";
  }

 private:
  TokenType    type_    { TokenType::NONE };
  TokenSubType subType_ { TokenSubType::NONE };
  std::string  str_;
};

using Tokens = std::vector<Token>;

//---

class Alias {
 public:
  Alias();
  Alias(const std::string &name, const StringArray &values) :
   name_(name), values_(values) {
  }

  const std::string &name() const { return name_; }
  void setName(const std::string &v) { name_ = v; }

  const StringArray &values() const { return values_; }
  void setValues(const StringArray &v) { values_ = v; }

 private:
  std::string name_;
  StringArray values_;
};

using AliasP = std::shared_ptr<Alias>;

//---

class BuiltinCommand;

class Cmd {
 public:
  Cmd(App *app);

  App *app() const { return app_; }

  const std::string &name() const { return name_; }
  void setName(const std::string &name) { name_ = name; }

  void addArg(const Token &arg) { args_.push_back(arg); }
  uint numArgs() const { return uint(args_.size()); }

  const Token &arg(uint i) const { return args_[i]; }

  void addOpt(const Token &opt, const std::string &arg);

  bool isPipe() const { return pipe_; }

  bool isFileInput() const { return ! input_.empty(); }
  bool isFileOutput() const { return ! output_.empty(); }

  const std::string &fileInput() const { return input_; }
  const std::string &fileOutput() const { return output_; }

  CCommand *command() const { return command_.get(); }

  void init() const;

  void start() const;

  void wait() const;

  void print(std::ostream &os) const;

 private:
  using CommandP = std::unique_ptr<CCommand>;

  App*            app_     { nullptr };
  std::string     name_;
  Tokens          args_;
  StringArray     ops_;
  CommandP        command_;
  BuiltinCommand* builtin_ { nullptr };
  bool            pipe_    { false };
  std::string     input_;
  std::string     output_;
};

using CmdP = std::shared_ptr<Cmd>;
using Cmds = std::vector<CmdP>;

//---

class Variable {
 public:
  Variable() { }

  Variable(const std::string &name, const std::string &value, bool exported=false) :
   name_(name), value_(value), exported_(exported) {
  }

  const std::string &name() const { return name_; }

  const std::string &value() const { return value_; }
  void setValue(const std::string &v) { value_ = v; }

  bool isExported() const { return exported_; }
  void setExported(bool b) { exported_ = b; }

 private:
  std::string name_;
  std::string value_;
  bool        exported_ { false };
};

using VariableMap = std::map<std::string, Variable>;

//---

class App;

class BuiltinCommand {
 public:
  BuiltinCommand(App *app) :
   app_(app) {
  }

  virtual ~BuiltinCommand() { }

  virtual void exec(const CmdP &cmd) = 0;

 protected:
  App *app_ { nullptr };
};

//---

class App {
 public:
  App();
 ~App();

  void init(int argc, char **argv);

  void addShellCommands();

  BuiltinCommand *getShellCommand(const std::string &name) const;

  void mainLoop();

  bool eof() const;

  std::string getLine() const;

  void parseLine(const std::string &line, Cmds &cmds) const;

  void lineToTokens(const std::string &line, Tokens &tokens) const;
  void tokensToCmds(const Tokens &tokens, Cmds &cmds) const;

  void expandToken(const Token &token, Tokens &tokens) const;

  bool expandArg(const Token &token, StringArray &args) const;

  bool expandVariable(const Token &token, std::string &arg1) const;
  bool expandStringVariable(const std::string &str, std::string &str1) const;

  std::string expandVarName(const std::string &name, bool bracketed) const;

  bool expandAlias(const std::string &name, StringArray &words) const;

  bool connectCommand(const CmdP &lastCmd, const CmdP &cmd, const CmdP &nextCmd);

  void processCommand(const CmdP &cmd);

  void waitCommand(const CmdP &cmd);

  std::string lookupDir(const std::string &dirname) const;

  bool changeDir(const std::string &dirname) const;

  void addVariable(const std::string &name, const std::string &value, bool exported=false);
  bool getVariable(const std::string &name, std::string &value) const;

  bool isKeyword(const std::string &name) const;

  void listAliases() const;
  void listAlias(const std::string &name) const;
  void addAlias(const std::string &name, const StringArray &values);

 private:
  using BuiltinCommandP = std::shared_ptr<BuiltinCommand>;

  using Commands = std::map<std::string, BuiltinCommandP>;
  using AliasMap = std::map<std::string, AliasP>;

  using ReadLineP = std::unique_ptr<CReadLine>;

  ReadLineP   readLine_;
  Commands    shellCommands_;
  VariableMap variableMap_;
  AliasMap    aliases_;
};

class CdCommand : public BuiltinCommand {
 public:
  CdCommand(App *app) :
   BuiltinCommand(app) {
  }

  void exec(const CmdP &cmd) override;
};

class ExitCommand : public BuiltinCommand {
 public:
  ExitCommand(App *app) :
   BuiltinCommand(app) {
  }

  void exec(const CmdP &cmd) override;
};

class AliasCommand : public BuiltinCommand {
 public:
  AliasCommand(App *app) :
   BuiltinCommand(app) {
  }

  void exec(const CmdP &cmd) override;
};

}

#endif
