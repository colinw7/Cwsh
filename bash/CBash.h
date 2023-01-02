#ifndef CBash_H
#define CBash_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <functional>
#include <memory>
#include <iostream>

class CCommand;

namespace CBash {

class App;
class ReadLine;
class History;

using StringArray = std::vector<std::string>;
using StringSet   = std::set<std::string>;

//---

enum class TokenType {
  NONE,
  SQUOTE_STRING,
  DQUOTE_STRING,
  BACKQUOTE_STRING,
  WORD,
  NUMBER,
  OPERATOR,
  KEYWORD
};

enum class Operator {
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
  PIPE_ERR,
  SEMI_COLON,
  SEMI_COLON_BG,
  DOUBLE_SEMI_COLON,
  DOUBLE_SEMI_COLON_BG,
  ASSIGN,
  BG,
  BG_GREATER,
  BG_GREATER_GREATER,
  OR,
  AND
};

enum class Keyword {
  IF,
  THEN,
  ELSE,
  ELIF,
  FI,
  CASE,
  ESAC,
  FOR,
  WHILE,
  UNTIL,
  DO,
  DONE,
  FUNCTION,
  IN,
  SELECT,
  NOT,
  OPEN_BRACE,
  CLOSE_BRACE,
  OPEN_BLOCK,
  CLOSE_BLOCK
};

class Token {
 public:
  Token() { }

  Token(const std::string &str) :
   str_(str) {
  }

  Token(TokenType type, const std::string &str, int pos) :
   type_(type), str_(str), pos_(pos) {
  }

  const TokenType &type() const { return type_; }

  void setSubType(int t) { subType_ = t; }

  Operator opType() const { return Operator(subType_); }

  Keyword keyword() const { return Keyword(subType_); }

  const std::string &str() const { return str_; }

  void print(std::ostream &os=std::cerr) const {
    switch (type_) {
      case TokenType::DQUOTE_STRING   : os << "\"string"; break;
      case TokenType::SQUOTE_STRING   : os << "\'string"; break;
      case TokenType::BACKQUOTE_STRING: os << "`string"; break;
      case TokenType::WORD            : os << "word"; break;
      case TokenType::NUMBER          : os << "number"; break;
      case TokenType::OPERATOR        : os << "op"; break;
      case TokenType::KEYWORD         : os << "keyword"; break;
      default                         : os << "none"; break;
    }

    os << "(" << str_ << ")";
    os << "\n";
  }

 private:
  TokenType    type_    { TokenType::NONE };
  int          subType_ { 0 };
  std::string  str_;
  int          pos_     { 0 };
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

using CCommandP = std::shared_ptr<CCommand>;

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
  bool isPipeErr() const { return pipe_ && pipeErr_; }

  bool isFileInput() const { return ! input_.empty(); }
  bool isFileOutput() const { return ! output_.empty(); }

  const std::string &fileInput() const { return input_; }
  const std::string &fileOutput() const { return output_; }

  CCommand *command() const { return command_.get(); }
  CCommandP commandp() const { return command_; }

  void init() const;

  void start() const;

  bool isWait() const { return wait_; }
  void wait() const;

  bool isAnd() const { return isAnd_; }

  bool isOr() const { return isOr_; }

  void print(std::ostream &os=std::cerr) const;

 private:
  App*            app_     { nullptr };
  std::string     name_;
  Tokens          args_;
  StringArray     ops_;
  bool            inited_  { false };
  CCommandP       command_;
  BuiltinCommand* builtin_ { nullptr };
  bool            pipe_    { false };
  bool            pipeErr_ { false };
  std::string     input_;
  std::string     output_;
  bool            wait_    { true };
  bool            isAnd_   { false };
  bool            isOr_    { false };
};

using CmdP = std::shared_ptr<Cmd>;
using Cmds = std::vector<CmdP>;

//---

class Variable {
 public:
  Variable() { }

  Variable(const std::string &name, const std::string &value="", bool exported=false) :
   name_(name), value_(value), exported_(exported) {
  }

  const std::string &name() const { return name_; }

  const std::string &value() const { return value_; }
  void setValue(const std::string &v) { value_ = v; }

  bool isExported() const { return exported_; }
  void setExported(bool b) { exported_ = b; }

  bool isReadOnly() const { return readOnly_; }
  void setReadOnly(bool b) { readOnly_ = b; }

  bool isTrace() const { return trace_; }
  void setTrace(bool b) { trace_ = b; }

 private:
  std::string name_;
  std::string value_;
  bool        exported_ { false };
  bool        readOnly_ { false };
  bool        trace_    { false };
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

  virtual void exec(const Cmd *cmd) = 0;

  void parseArgs(const Cmd *cmd);

  virtual bool processOpt(const std::string &opt);
  virtual bool processArg(const std::string &arg);

 protected:
  App*        app_ { nullptr };
  StringArray args_;
};

//---

class App {
 public:
  App();
 ~App();

  bool isDebug() const { return debug_; }

  void init(int argc, char **argv);

  void addShellCommands();

  BuiltinCommand *getShellCommand(const std::string &name) const;

  void mainLoop();

  bool eof() const;

  std::string getLine(bool &complete) const;

  std::string getInputPrompt() const;

  std::string adjustPrompt(const std::string &prompt) const;

  bool parseLine(const std::string &line, Cmds &cmds) const;

  bool lineToTokens(const std::string &line, Tokens &tokens) const;

  bool tokensToCmds(const Tokens &tokens, Cmds &cmds) const;

  bool expandToken(const Token &token, Tokens &tokens) const;

  bool expandArg(const Token &token, StringArray &args) const;

  bool expandVariableToken(const Token &token, std::string &arg1) const;
  bool expandStringVariable(const std::string &str, std::string &str1) const;

  std::string expandVarName(const std::string &name, bool bracketed) const;

  bool expandAlias(const std::string &name, StringArray &words) const;

  bool connectCommand(const CmdP &lastCmd, const CmdP &cmd, const CmdP &nextCmd);

  void processCommand(const CmdP &cmd);

  void waitCommand(const CmdP &cmd);

  void checkBgCommands();

  //--

  int returnCode() const { return returnCode_; }
  void setReturnCode(int i) { returnCode_ = i; }

  //--

  History *history() { return history_.get(); }

  void addHistory(const std::string &line);
  void listHistory() const;

  //---

  int commandNum() const { return commandNum_; }

  //---

  std::string lookupDir(const std::string &dirname) const;

  bool changeDir(const std::string &dirname) const;

  //---

  void addVariable(const std::string &name, const std::string &value, bool exported=false);
  void addVariable(const std::string &name, const Variable &var);

  void removeVariable(const std::string &name);

  bool getVariable(const std::string &name, std::string &value) const;

  bool completeVariable(const std::string &name, std::string &expandedName) const;

  bool showMatchingVariables(const std::string &name)  const;

  struct ListVariablesData {
    bool exported { false };
    bool declare  { false };
  };

  void listVariables(const ListVariablesData &data) const;

  //---

  bool isKeyword(const std::string &name, Keyword &keyword) const;

  void listAliases() const;
  void listAlias(const std::string &name) const;
  void addAlias(const std::string &name, const StringArray &values);

  //---

  bool hasDirStack() const;
  void clearDirStack();
  void pushDirStack(const std::string &dir);
  std::string rotateDirStack();
  std::string popDirStack();
  void listDirStack(bool /*verbose*/) const;

  //---

  void addSignalHandlers();

  //---

  void printWords(const StringArray &words) const;

 private:
  static void genericHandler(int sig);
  static void interruptHandler(int sig);
  static void termHandler(int sig);

 private:
  using ReadLineP       = std::unique_ptr<ReadLine>;
  using HistoryP        = std::unique_ptr<History>;
  using BuiltinCommandP = std::shared_ptr<BuiltinCommand>;
  using Commands        = std::map<std::string, BuiltinCommandP>;
  using AliasMap        = std::map<std::string, AliasP>;
  using BgCommands      = std::set<CCommandP>;
  using DirStack        = std::vector<std::string>;

  bool        debug_ { false };
  ReadLineP   readLine_;
  bool        complete_ { true };
  Commands    shellCommands_;
  VariableMap variableMap_;
  AliasMap    aliases_;
  HistoryP    history_;
  int         commandNum_ { 1 };
  BgCommands  bgCommands_;
  DirStack    dirStack_;
  int         returnCode_ { 0 };
};

}

//---

#include <CReadLine.h>

namespace CBash {

class ReadLine : public CReadLine {
 public:
  ReadLine(App *app);

  std::string readLine();

  void beep() override;
  void interrupt() override;
  void timeout() override;

 private:
  bool completeLine(const std::string &line, std::string &line1) override;

  bool showComplete(const std::string &line) override;

  bool getPrevCommand(std::string &line) override;
  bool getNextCommand(std::string &line) override;

 private:
  App* app_ { nullptr };
};

}

//---

#include <CHistory.h>

namespace CBash {

class History {
 public:
  History(App *app);

  void addCommand(const std::string &line);

  bool hasPrevCommand();
  bool hasNextCommand();

  std::string getPrevCommand();
  std::string getNextCommand();

  void display(int num, bool show_numbers, bool show_time, bool reverse);

  int commandNum() const { return commandNum_; }

 private:
  App*     app_ { nullptr };
  CHistory history_;
  int      commandNum_ { 0 };
};

}

//---

namespace CBash {

class AliasCommand : public BuiltinCommand {
 public:
  AliasCommand(App *app) :
   BuiltinCommand(app) {
  }

  void exec(const Cmd *cmd) override;
};

class CdCommand : public BuiltinCommand {
 public:
  CdCommand(App *app) :
   BuiltinCommand(app) {
  }

  void exec(const Cmd *cmd) override;
};

class DeclareCommand : public BuiltinCommand {
 public:
  DeclareCommand(App *app) :
   BuiltinCommand(app) {
  }

  void exec(const Cmd *cmd) override;
};

class DirsCommand : public BuiltinCommand {
 public:
  DirsCommand(App *app) :
   BuiltinCommand(app) {
  }

  void exec(const Cmd *cmd) override;

  bool processOpt(const std::string &opt) override;

 private:
  bool clear_   { false };
  bool list_    { false };
  bool print_   { false };
  bool verbose_ { false };
};

class EchoCommand : public BuiltinCommand {
 public:
  EchoCommand(App *app) :
   BuiltinCommand(app) {
  }

  void exec(const Cmd *cmd) override;

  bool processOpt(const std::string &opt) override;

 private:
  bool nonewline_    { false };
  bool enableEscape_ { false };
};

class ExitCommand : public BuiltinCommand {
 public:
  ExitCommand(App *app) :
   BuiltinCommand(app) {
  }

  void exec(const Cmd *cmd) override;
};

class ExportCommand : public BuiltinCommand {
 public:
  ExportCommand(App *app) :
   BuiltinCommand(app) {
  }

  void exec(const Cmd *cmd) override;

  bool processOpt(const std::string &opt) override;

 private:
  bool function_ { false };
  bool unexport_ { false };
  bool display_  { false };
};

class HistoryCommand : public BuiltinCommand {
 public:
  HistoryCommand(App *app) :
   BuiltinCommand(app) {
  }

  void exec(const Cmd *cmd) override;
};

class PopdCommand : public BuiltinCommand {
 public:
  PopdCommand(App *app) :
   BuiltinCommand(app) {
  }

  void exec(const Cmd *cmd) override;
};

class PrintfCommand : public BuiltinCommand {
 public:
  PrintfCommand(App *app) :
   BuiltinCommand(app) {
  }

  void exec(const Cmd *cmd) override;
};

class PushdCommand : public BuiltinCommand {
 public:
  PushdCommand(App *app) :
   BuiltinCommand(app) {
  }

  void exec(const Cmd *cmd) override;
};

class PwdCommand : public BuiltinCommand {
 public:
  PwdCommand(App *app) :
   BuiltinCommand(app) {
  }

  void exec(const Cmd *cmd) override;

  bool processOpt(const std::string &opt) override;

 private:
  bool list_  { false };
  bool print_ { false };
};

class SetCommand : public BuiltinCommand {
 public:
  SetCommand(App *app) :
   BuiltinCommand(app) {
  }

  void exec(const Cmd *cmd) override;
};

}

#endif
