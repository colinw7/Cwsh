#include <CBash.h>
#include <CReadLine.h>
#include <CCommand.h>
#include <CStrParse.h>
#include <CFile.h>
#include <CFileMatch.h>
#include <CDir.h>
#include <CGlob.h>
#include <CStrUtil.h>
#include <COSProcess.h>
#include <COSUser.h>
#include <CEnv.h>
#include <CExpr.h>

namespace CBash {

App::
App()
{
  readLine_ = std::make_unique<CReadLine>();
}

App::
~App()
{
}

void
App::
init(int, char **)
{
  addShellCommands();

  for (const auto &env : CEnvInst)
    addVariable(env.first, env.second, /*export*/true);
}

void
App::
addShellCommands()
{
  shellCommands_["cd"   ] = std::make_shared<CdCommand   >(this);
  shellCommands_["alias"] = std::make_shared<AliasCommand>(this);
  shellCommands_["exit" ] = std::make_shared<ExitCommand >(this);
}

BuiltinCommand *
App::
getShellCommand(const std::string &name) const
{
  auto p = shellCommands_.find(name);
  if (p == shellCommands_.end()) return nullptr;

  return (*p).second.get();
}

void
App::
mainLoop()
{
  while (! eof()) {
    auto line = getLine();

    Cmds cmds;

    parseLine(line, cmds);

    //---

    size_t nc = cmds.size();

    for (size_t ic = 0; ic < nc; ++ic) {
      auto lastCmd = (ic > 0      ? cmds[ic - 1] : CmdP());
      auto nextCmd = (ic < nc - 1 ? cmds[ic + 1] : CmdP());

      connectCommand(lastCmd, cmds[ic], nextCmd);
    }

    //---

    for (const auto &cmd : cmds)
      processCommand(cmd);

    for (const auto &cmd : cmds)
      waitCommand(cmd);
  }
}

void
App::
parseLine(const std::string &line, Cmds &cmds) const
{
  Tokens tokens;

  lineToTokens(line, tokens);

  tokensToCmds(tokens, cmds);
}

void
App::
lineToTokens(const std::string &line, Tokens &tokens) const
{
  // TODO:
  // # comment (ignore to end of line)

  CStrParse parse(line);

  parse.skipSpace();

  if (parse.eof())
    return;

  auto type    = TokenType::NONE;
  auto subType = TokenSubType::NONE;
  auto pos     = parse.getPos();

  //---

  auto flushToken = [&](bool skip=true) {
    auto word = parse.getBefore(pos);

    if (word != "") {
      if (type == TokenType::NONE && isKeyword(word))
        type = TokenType::KEYWORD;

      Token token(type, word);

      token.setSubType(subType);

      tokens.push_back(token);

      std::cerr << "Token: ";

      tokens.back().print(std::cerr);
    }

    if (skip)
      parse.skipSpace();

    type = TokenType::NONE;
    pos  = parse.getPos();
  };

  //---

  auto skipWord = [&]() {
    parse.skipSpace();

    while (! parse.eof()) {
      if      (parse.isSpace())
        break;
      else if (parse.isChar(';'))
        break;
      else if (parse.isChar('|') && parse.isChar('&'))
        break;
      else if (parse.isChar('(') || parse.isChar(')'))
        break;
      else if (parse.isChar('<') || parse.isChar('>'))
        break;
      else
        parse.skipChar();
    }

    if (parse.getPos() > pos)
      type = TokenType::STRING;
  };

  //---

  while (! parse.eof()) {
    if      (parse.isChar('"')) {
      flushToken(/*skip*/false);

      type = TokenType::STRING;

      if (! parse.skipString()) {
        throw "Invalid string";
        break; // error
      }

      flushToken();
    }
    else if (parse.isChar('\'')) {
      flushToken(/*skip*/false);

      type = TokenType::STRING;

      if (! parse.skipString()) {
        throw "Invalid string";
        break; // error
      }

      flushToken();
    }
    else if (parse.isChar('`')) {
      flushToken(/*skip*/false);

      type = TokenType::STRING;

      if (! parse.skipString()) {
        throw "Invalid string";
        break; // error
      }

      flushToken();
    }
    else if (parse.isChar('(') || parse.isChar(')')) {
      flushToken(/*skip*/false);

      type = TokenType::OPERATOR;

      subType = (parse.isChar('(') ? TokenSubType::OPEN_RBRACKETS : TokenSubType::CLOSE_RBRACKETS);

      parse.skipChar();

      flushToken();
    }
    // <, << or <<-, <&, <>
    else if (parse.isChar('<')) {
      flushToken(/*skip*/false);

      type = TokenType::OPERATOR;

      parse.skipChar();

      if      (parse.isChar('<')) {
        parse.skipChar();

        if (parse.isChar('-')) {
          parse.skipChar();

          subType = TokenSubType::LESS_LESS_MINUS;
        }
        else
          subType = TokenSubType::LESS_LESS;
      }
      else if (parse.isChar('&')) {
        parse.skipChar();

        subType = TokenSubType::FILE_INPUT_ERR;
      }
      else if (parse.isChar('>')) {
        parse.skipChar();

        subType = TokenSubType::LESS_GREATER;
      }
      else
        subType = TokenSubType::FILE_INPUT;

      flushToken();
    }
    // >, >>, >&, >|
    else if (parse.isChar('>')) {
      flushToken(/*skip*/false);

      type = TokenType::OPERATOR;

      parse.skipChar();

      if      (parse.isChar('>')) {
        parse.skipChar();

        subType = TokenSubType::FILE_OUTPUT_APPEND;
      }
      else if (parse.isChar('&')) {
        parse.skipChar();

        subType = TokenSubType::FILE_OUTPUT_ERR;
      }
      else if (parse.isChar('|')) {
        parse.skipChar();

        subType = TokenSubType::FILE_OUTPUT_PIPE;
      }
      else
        subType = TokenSubType::FILE_OUTPUT;

      flushToken();

      skipWord();

      flushToken();
    }
    // |, ||, |&
    else if (parse.isChar('|')) {
      flushToken(/*skip*/false);

      type = TokenType::OPERATOR;

      parse.skipChar();

      if      (parse.isChar('|')) {
        parse.skipChar();

        subType = TokenSubType::PIPE_PIPE;
      }
      else if (parse.isChar('&')) {
        parse.skipChar();

        subType = TokenSubType::PIPE_ERR;
      }
      else
        subType = TokenSubType::PIPE;

      flushToken();
    }
    // &, &&, &>, &>>
    else if (parse.isChar('&')) {
      flushToken(/*skip*/false);

      type = TokenType::OPERATOR;

      parse.skipChar();

      if      (parse.isChar('&'))
        parse.skipChar();
      else if (parse.isChar('>')) {
        parse.skipChar();

        if (parse.isChar('>'))
          parse.skipChar();
      }

      flushToken();
    }
    // ;, ;;, ;;&, ;&
    else if (parse.isChar(';')) {
      flushToken(/*skip*/false);

      type = TokenType::OPERATOR;

      parse.skipChar();

      if      (parse.isChar(';')) {
        parse.skipChar();

        if (parse.isChar('&')) {
          parse.skipChar();

          subType = TokenSubType::DOUBLE_SEMI_COLON_BG;
        }
        else
          subType = TokenSubType::DOUBLE_SEMI_COLON;
      }
      else if (parse.isChar('&')) {
        parse.skipChar();

        subType = TokenSubType::SEMI_COLON_BG;
      }
      else
        subType = TokenSubType::SEMI_COLON;

      flushToken();
    }
    else if (parse.isChar('$')) {
      parse.skipChar();

      if (parse.isChar('(')) {
        parse.skipChar();

        int brackets = 1;

        while (! parse.eof()) {
          if      (parse.isChar('(')) {
            parse.skipChar();

            ++brackets;
          }
          else if (parse.isChar(')')) {
            parse.skipChar();

            if (brackets == 1)
              break;

            --brackets;
          }
          else
            parse.skipChar();
        }
      }
    }
    else if (parse.isChar('=')) {
      if (type == TokenType::STRING) {
        flushToken();

        type = TokenType::OPERATOR;

        parse.skipChar();

        flushToken();
      }
      else
        parse.skipChar();
    }
    else if (parse.isSpace()) {
      flushToken();

      type = TokenType::NONE;
    }
    else if (parse.isAlpha() || parse.isChar('_')) {
      if (type == TokenType::NONE) {
        while (parse.isAlnum() || parse.isChar('_'))
          parse.skipChar();

        type = TokenType::STRING;
      }
      else {
        parse.skipChar();
      }
    }
    else if (parse.isDigit()) {
      if (type == TokenType::NONE) {
        while (parse.isDigit())
          parse.skipChar();

        type = TokenType::NUMBER;
      }
    }
    else {
      parse.skipChar();
    }
  }

  flushToken();
}

void
App::
tokensToCmds(const Tokens &tokens, Cmds &cmds) const
{
  auto *th = const_cast<App *>(this);

  int brackets = 0;

  CmdP cmd = std::make_shared<Cmd>(th);

  auto flushCmd = [&]() {
    if (cmd->name() != "") {
      cmds.push_back(cmd);

      cmd->print(std::cerr);
    }

    cmd = std::make_shared<Cmd>(th);
  };

  size_t it = 0;
  auto   nt = tokens.size();

  while (it < nt) {
    const auto &token = tokens[it++];

    const std::string &str = token.str();

    if (token.type() == TokenType::OPERATOR) {
      auto subType = token.subType();

      if      (subType == TokenSubType::OPEN_RBRACKETS) {
        ++brackets;
      }
      else if (subType == TokenSubType::CLOSE_RBRACKETS) {
        if (brackets == 0) {
          throw "bad ()";
          break; // error
        }

        --brackets;
      }
      else if (subType == TokenSubType::SEMI_COLON) {
        if (brackets == 0)
          flushCmd();
      }
      else if (subType == TokenSubType::FILE_INPUT || subType == TokenSubType::FILE_INPUT_ERR ||
               subType == TokenSubType::FILE_OUTPUT || subType == TokenSubType::FILE_OUTPUT_ERR) {
        auto fileStr = (it < nt ? tokens[it++].str() : "");

        cmd->addOpt(token, fileStr);
      }
      else {
        cmd->addOpt(token, "");

        if (subType == TokenSubType::PIPE)
          flushCmd();
      }
    }
    else {
      if (cmd->name() == "") {
        StringArray words;

        if (expandAlias(str, words)) {
          for (const auto &word : words) {
            if (cmd->name() == "")
              cmd->setName(word);
            else
              cmd->addArg(word);
          }
        }
        else
          cmd->setName(str);
      }
      else {
        Tokens tokens1;

        expandToken(token, tokens1);

        for (const auto &token1 : tokens1)
          cmd->addArg(token1);
       }
    }
  }

  if (brackets != 0) {
    throw "bad ()";
    return;
  }

  flushCmd();
}

bool
App::
expandAlias(const std::string &name, StringArray &args) const
{
  auto pa = aliases_.find(name);
  if (pa == aliases_.end()) return false;

  const auto &alias = (*pa).second;

  for (const auto &value : alias->values()) {
    Tokens tokens1;

    lineToTokens(value, tokens1);

    for (const auto &token1 : tokens1)
      args.push_back(token1.str());
  }

  return true;
}

void
App::
expandToken(const Token &token, Tokens &tokens) const
{
  const std::string &str = token.str();

  // brace expansion
  CFileMatch fm;

  StringArray args1;

  if (fm.expandBraces(str, args1)) {
    for (const auto &arg1 : args1) {
      StringArray args2;

      if (expandArg(arg1, args2)) {
        for (const auto &arg2 : args2)
          tokens.push_back(arg2);
      }
      else
        tokens.push_back(arg1);
    }
  }
  else {
    if (expandArg(token, args1)) {
      for (const auto &arg1 : args1)
        tokens.push_back(arg1);
    }
    else
      tokens.push_back(token);
  }
}

bool
App::
expandArg(const Token &token, StringArray &args) const
{
  bool changed = false;

  auto token1 = token;

  //---

  // expand tilde (~)
  {
  auto str1 = token1.str();

  std::string arg2;

  if (CFile::expandTilde(str1, arg2)) {
    token1 = arg2;
    changed = true;
  }
  }

  //---

  // expand variables
  {
  auto str1 = token1.str();

  if (token1.type() == TokenType::STRING) {
    if (str1[0] == '\"') {
      std::string str2;

      if (expandStringVariable(str1, str2)) {
        token1 = str2;

        changed = true;
      }
    }
  }
  else {
    if (str1[0] == '$') {
      std::string arg2;

      if (expandVariable(token1, arg2)) {
        token1 = arg2;

        changed = true;
      }
    }
  }
  }

  //---

  {
  auto str1 = token1.str();

  CGlob glob(str1);

  if (glob.isPattern()) {
    glob.setAllowOr(false);
    glob.setAllowNonPrintable(true);

    StringArray files1;

    CFileMatch fileMatch;

    if (fileMatch.matchPattern(str1, files1)) {
      CStrUtil::sort(files1);

      for (const auto &file1 : files1)
        args.push_back(file1);

      changed = true;
    }
  }
  else {
    if (changed)
      args.push_back(str1);
  }
  }

  //---

  return changed;
}

bool
App::
expandVariable(const Token &token, std::string &arg1) const
{
  bool changed = false;

  auto arg = token.str();

  arg1 = "";

  size_t i   = 0;
  auto   len = arg.size();

  assert(arg[i] == '$');

  ++i;

  // $$
  if      (arg[i] == '$') {
    arg1 = expandVarName("$", /*bracketed*/false) + arg.substr(2);

    changed = true;
  }
  // $(<var>)
  else if (arg[i] == '(') {
    ++i;

    if (arg[i] == '(') {
      ++i;

      auto j = i;

      int brackets = 2;

      while (i < len) {
        if      (arg[i] == ')') {
          if (brackets == 1)
            break;

          --brackets;
        }
        else if (arg[i] == '(') {
          ++brackets;
        }

        ++i;
      }

      if (i >= len)
        throw "Bad brackets for variable";

      auto name = arg.substr(j, i - j - 1);

      arg1 = expandVarName(name, /*bracketed*/true);

      changed = true;
    }
    else {
      auto j = i;

      int brackets = 1;

      while (i < len) {
        if      (arg[i] == ')') {
          if (brackets == 1)
            break;

          --brackets;
        }
        else if (arg[i] == '(') {
          ++brackets;
        }

        ++i;
      }

      if (i >= len)
        throw "Bad brackets for variable";

      auto name = arg.substr(j, i - j);

      arg1 = expandVarName(name, /*bracketed*/false);

      changed = true;
    }
  }
  // $<var>
  else {
    auto name = arg.substr(1);

    arg1 = expandVarName(name, /*bracketed*/false);

    changed = true;
  }

  return changed;
}

bool
App::
expandStringVariable(const std::string &str, std::string &str1) const
{
  bool changed = false;

  CStrParse parse(str);

  while (! parse.eof()) {
    if (parse.isChar('$')) {
      parse.skipChar();

      if (parse.isChar('(')) {
        parse.skipChar();

        // $((<word>))
        if (parse.isChar('(')) {
          int brackets = 2;

          int pos = parse.getPos();

          while (! parse.eof()) {
            if      (parse.isChar(')')) {
              if (brackets == 1)
                break;

              --brackets;
            }
            else if (parse.isChar('(')) {
              ++brackets;
            }

            parse.skipChar();
          }

          auto word = parse.getBefore(pos);

          str1 += expandVarName(word, /*bracketed*/false);

          changed = true;
        }
        // $(<word>)
        else {
          int brackets = 1;

          int pos = parse.getPos();

          while (! parse.eof()) {
            if      (parse.isChar(')')) {
              if (brackets == 1)
                break;

              --brackets;
            }
            else if (parse.isChar('(')) {
              ++brackets;
            }

            parse.skipChar();
          }

          auto word = parse.getBefore(pos);

          str1 += expandVarName(word, /*bracketed*/false);

          changed = true;
        }
      }
      // $<word>
      else {
        int pos = parse.getPos();

        while (! parse.eof()) {
          if (parse.isChar('\"') || parse.isSpace())
            break;

          parse.skipChar();
        }

        auto word = parse.getBefore(pos);

        str1 += expandVarName(word, /*bracketed*/false);

        changed = true;
      }
    }
    else {
      str1 += parse.readChar();
    }
  }

  return changed;
}

std::string
App::
expandVarName(const std::string &name, bool bracketed) const
{
  if      (bracketed) {
    StringArray names1;

    if (expandArg(name, names1)) {
      for (const auto &name1 : names1) {
        std::string value;

        CExpr expr;

        CExprValuePtr evalue;

        if (! expr.evaluateExpression(name1, evalue))
          throw "Bad expression";

        std::string s;

        if (evalue && evalue->getStringValue(s))
          return s;
      }
    }
  }
  else if (name == "$") {
    int pid = COSProcess::getProcessId();

    return CStrUtil::toString(pid);
  }
  else {
    std::string value;

    if (getVariable(name, value))
      return value;
  }

  return name;
}

bool
App::
eof() const
{
  return false;
}

std::string
App::
getLine() const
{
  auto line = readLine_->readLine();

  return line;
}

bool
App::
connectCommand(const CmdP & /*lastCmd*/, const CmdP &cmd, const CmdP &nextCmd)
{
  // connect this command to next
  if      (cmd->isPipe()) {
    if (! nextCmd)
      return false;

    cmd    ->init();
    nextCmd->init();

    if (cmd->command() && nextCmd->command()) {
      cmd    ->command()->addPipeDest(1);
      nextCmd->command()->addPipeSrc();
    }
  }
  else if (cmd->isFileOutput()) {
    cmd->init();

    cmd->command()->addFileDest(cmd->fileOutput());
  }
  else if (cmd->isFileInput()) {
    cmd->init();

    cmd->command()->addFileSrc(cmd->fileInput());
  }

  return true;
}

void
App::
processCommand(const CmdP &cmd)
{
  cmd->init();

  cmd->start();
}

void
App::
waitCommand(const CmdP &cmd)
{
  cmd->wait();
}

std::string
App::
lookupDir(const std::string &dirname) const
{
  if (CFile::exists(dirname) && CFile::isDirectory(dirname))
    return dirname;

  return "";
}

bool
App::
changeDir(const std::string &dirname) const
{
  if (! CDir::changeDir(dirname))
    return false;

  return true;
}

//---

void
App::
addVariable(const std::string &name, const std::string &value, bool exported)
{
  variableMap_[name] = Variable(name, value, exported);
}

bool
App::
getVariable(const std::string &name, std::string &value) const
{
  auto p = variableMap_.find(name);

  if (p == variableMap_.end())
    return false;

  value = (*p).second.value();

  return true;
}

//---

bool
App::
isKeyword(const std::string &name) const
{
  static StringSet keywords;

  if (keywords.empty()) {
    keywords.insert("if");
    keywords.insert("then");
    keywords.insert("else");
    keywords.insert("elif");
    keywords.insert("fi");

    keywords.insert("case");
    keywords.insert("esac");

    keywords.insert("for");
    keywords.insert("while");
    keywords.insert("until");
    keywords.insert("do");
    keywords.insert("done");

    keywords.insert("function");
    keywords.insert("in");

    keywords.insert("select");

    keywords.insert("!");
    keywords.insert("{");
    keywords.insert("}");
  }

  return (keywords.find(name) != keywords.end());
}

//---

void
App::
listAliases() const
{
  for (const auto &pa : aliases_) {
    const auto &alias = pa.second;

    std::cerr << alias->name() << " =";

    for (const auto &value : alias->values())
      std::cerr << " " << value;

    std::cerr << "\n";
  }
}

void
App::
listAlias(const std::string &name) const
{
  auto pa = aliases_.find(name);
  if (pa == aliases_.end()) return;

  const auto &alias = (*pa).second;

  std::cerr << alias->name() << " =";

  for (const auto &value : alias->values())
    std::cerr << " " << value;

  std::cerr << "\n";
}

void
App::
addAlias(const std::string &name, const StringArray &values)
{
  aliases_[name] = std::make_shared<Alias>(name, values);
}

//---

void
CdCommand::
exec(const CmdP &cmd)
{
  auto num_args = cmd->numArgs();

  if (num_args == 0) {
    auto dirname = COSUser::getUserHome();

    app_->changeDir(dirname);
  }
  else {
    for (uint i = 0; i < num_args; ++i) {
      const auto &token = cmd->arg(i);

      auto dirname = CStrUtil::stripSpaces(token.str());

      if (dirname == "")
        dirname = COSUser::getUserHome();

      dirname = app_->lookupDir(dirname);

      app_->changeDir(dirname);
    }
  }
}

//---

void
AliasCommand::
exec(const CmdP &cmd)
{
  auto num_args = cmd->numArgs();

  if (num_args == 0) {
    cmd->app()->listAliases();
  }
  else {
    const auto &token = cmd->arg(0);

    const auto &arg = token.str();

    if (arg[arg.size() - 1] == '=') {
      auto name = arg.substr(0, arg.size() - 1);

      StringArray values;

      for (uint i = 1; i < num_args; ++i) {
        const auto &token1 = cmd->arg(i);

        if (token1.type() == TokenType::STRING)
          values.push_back(token1.sstr());
        else
          values.push_back(token1.str());
      }

      cmd->app()->addAlias(name, values);
    }
    else {
      for (uint i = 0; i < num_args; ++i) {
        const auto &token1 = cmd->arg(i);

        cmd->app()->listAlias(token1.str());
      }
    }
  }
}

//---

void
ExitCommand::
exec(const CmdP &)
{
  exit(0);
}

//---

Cmd::
Cmd(App *app) :
 app_(app)
{
}

void
Cmd::
addOpt(const Token &token, const std::string &arg)
{
  auto subType = token.subType();

  assert(subType != TokenSubType::NONE);

  if      (subType == TokenSubType::PIPE || subType == TokenSubType::PIPE_ERR)
    pipe_ = true;
  else if (subType == TokenSubType::FILE_OUTPUT || subType == TokenSubType::FILE_OUTPUT_ERR)
    output_ = arg;
  else if (subType == TokenSubType::FILE_INPUT || subType == TokenSubType::FILE_INPUT_ERR)
    input_ = arg;

  auto str = token.str();

  ops_.push_back(str);
}

void
Cmd::
init() const
{
  if (! builtin_ && ! command_) {
    auto *th = const_cast<Cmd *>(this);

    th->builtin_ = app_->getShellCommand(name_);

    if (! builtin_) {
      StringArray args;

      for (const auto &arg : args_)
        args.push_back(arg.str());

      th->command_ = std::make_unique<CCommand>(name_, name_, args);
    }
  }
}

void
Cmd::
start() const
{
  if (command_)
    command_->start();
}

void
Cmd::
wait() const
{
  if (command_)
    command_->wait();
}

void
Cmd::
print(std::ostream &os) const
{
  os << name_;

  for (const auto &arg : args_)
    os << " " << arg.str();

  os << "\n";
}

//---

}
