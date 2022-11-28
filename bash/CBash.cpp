#include <CBash.h>
#include <CReadLine.h>
#include <CCommand.h>
#include <CStrParse.h>
#include <CFile.h>
#include <CFileMatch.h>
#include <CPathList.h>
#include <CDir.h>
#include <CGlob.h>
#include <CStrUtil.h>
#include <CPrintF.h>

#include <COSFile.h>
#include <COSProcess.h>
#include <COSSignal.h>
#include <COSTerm.h>
#include <COSTime.h>
#include <COSUser.h>

#include <CEnv.h>
#include <CExpr.h>

namespace CBash {

App::
App()
{
  readLine_ = std::make_unique<ReadLine>(this);
  history_  = std::make_unique<History >(this);
}

App::
~App()
{
}

void
App::
init(int argc, char **argv)
{
  StringArray args;

  for (int i = 1; i < argc; ++i) {
    if (argv[i][1] == '-') {
      auto opt = std::string(&argv[i][1]);

      if (opt == "-debug")
        debug_ = true;
      else
        std::cerr << "Invalid option '-" << opt << "\n";
    }
    else
      args.push_back(argv[i]);
  }

  addShellCommands();

  //---

  addVariable("PS1", "$ ");
  addVariable("PS2", "> ");

  for (const auto &env : CEnvInst)
    addVariable(env.first, env.second, /*export*/true);

  //---

  addSignalHandlers();
}

void
App::
addShellCommands()
{
//shellCommands_[":"        ] = std::make_shared<ColonCommand    >(this);
//shellCommands_["."        ] = std::make_shared<DotCommand      >(this);
  shellCommands_["alias"    ] = std::make_shared<AliasCommand    >(this);
//shellCommands_["bind"     ] = std::make_shared<BindCommand     >(this);
//shellCommands_["break"    ] = std::make_shared<BreakCommand    >(this);
//shellCommands_["builtin"  ] = std::make_shared<BuiltinCommand  >(this);
//shellCommands_["caller"   ] = std::make_shared<CallerCommand   >(this);
  shellCommands_["cd"       ] = std::make_shared<CdCommand       >(this);
//shellCommands_["command"  ] = std::make_shared<CommandCommand  >(this);
//shellCommands_["compgen"  ] = std::make_shared<CommandCommand  >(this);
//shellCommands_["continue" ] = std::make_shared<ContinueCommand >(this);
//shellCommands_["declare"  ] = std::make_shared<DeclareCommand  >(this);
//shellCommands_["echo"     ] = std::make_shared<EchoCommand     >(this);
//shellCommands_["enable"   ] = std::make_shared<EnableCommand   >(this);
//shellCommands_["eval"     ] = std::make_shared<EvalCommand     >(this);
//shellCommands_["exec"     ] = std::make_shared<ExecCommand     >(this);
  shellCommands_["exit"     ] = std::make_shared<ExitCommand     >(this);
//shellCommands_["export"   ] = std::make_shared<ExportCommand   >(this);
//shellCommands_["getopts"  ] = std::make_shared<GetoptsCommand  >(this);
//shellCommands_["hash"     ] = std::make_shared<HashCommand     >(this);
//shellCommands_["help"     ] = std::make_shared<HelpCommand     >(this);
  shellCommands_["history"  ] = std::make_shared<HistoryCommand  >(this);
//shellCommands_["let"      ] = std::make_shared<LetCommand      >(this);
//shellCommands_["local"    ] = std::make_shared<LocalCommand    >(this);
//shellCommands_["logout"   ] = std::make_shared<LogoutCommand   >(this);
//shellCommands_["mapfile"  ] = std::make_shared<MapfileCommand  >(this);
  shellCommands_["printf"   ] = std::make_shared<PrintfCommand   >(this);
//shellCommands_["pwd"      ] = std::make_shared<PwdCommand      >(this);
//shellCommands_["read"     ] = std::make_shared<ReadCommand     >(this);
//shellCommands_["readarray"] = std::make_shared<ReadArrayCommand>(this);
//shellCommands_["readonly" ] = std::make_shared<ReadonlyCommand >(this);
//shellCommands_["return"   ] = std::make_shared<ReturnCommand   >(this);
  shellCommands_["set"      ] = std::make_shared<SetCommand      >(this);
//shellCommands_["shift"    ] = std::make_shared<ShiftCommand    >(this);
//shellCommands_["shopt"    ] = std::make_shared<ShoptCommand    >(this);
//shellCommands_["source"   ] = std::make_shared<SourceCommand   >(this);
//shellCommands_["test"     ] = std::make_shared<TestCommand     >(this);
//shellCommands_["times"    ] = std::make_shared<TimesCommand    >(this);
//shellCommands_["trap"     ] = std::make_shared<TrapCommand     >(this);
//shellCommands_["type"     ] = std::make_shared<TypeCommand     >(this);
//shellCommands_["typeset"  ] = std::make_shared<TypesetCommand  >(this);
//shellCommands_["ulimit"   ] = std::make_shared<UlimitCommand   >(this);
//shellCommands_["unalias"  ] = std::make_shared<UnaliasCommand  >(this);
//shellCommands_["umask"    ] = std::make_shared<UmaskCommand    >(this);
//shellCommands_["unset"    ] = std::make_shared<UnsetCommand    >(this);
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
  std::string lastLine;

  complete_ = true;

  while (! eof()) {
    checkBgCommands();

    //---

    setReturnCode(0);

    //---

    auto line = getLine(complete_);

    line = lastLine + " " + line;

    if (! complete_) {
      lastLine = line;
      continue;
    }

    lastLine = "";

    Cmds cmds;

    if (! parseLine(line, cmds))
      continue;

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

    //---

    addHistory(line);

    ++commandNum_;
  }
}

bool
App::
parseLine(const std::string &line, Cmds &cmds) const
{
  // Skip if comment as first character
  uint i = 0;

  CStrUtil::skipSpace(line, &i);

  uint len = uint(line.size());

  if (i < len && line[i] == '#')
    return true;

  //---

  Tokens tokens;

  try {
    if (! lineToTokens(line, tokens))
      return false;

    if (! tokensToCmds(tokens, cmds))
      return false;
  }
  catch (...) {
    return false;
  }

  return true;
}

bool
App::
lineToTokens(const std::string &line, Tokens &tokens) const
{
  // TODO:
  // # comment (ignore to end of line)

  CStrParse parse(line);

  parse.skipSpace();

  if (parse.eof())
    return false;

  auto        type    = TokenType::NONE;
  int         subType = 0;
  auto        pos     = parse.getPos();
  std::string word    = "";

  //---

  auto flushToken = [&](bool skip=true) {
    if (word != "") {
      Keyword keyword;

      if (type == TokenType::NONE && isKeyword(word, keyword)) {
        type    = TokenType::KEYWORD;
        subType = int(keyword);
      }

      Token token(type, word, pos);

      token.setSubType(subType);

      tokens.push_back(token);

      if (isDebug()) {
        std::cerr << "Token: ";

        tokens.back().print(std::cerr);
      }
    }

    if (skip)
      parse.skipSpace();

    type = TokenType::NONE;
    pos  = parse.getPos();
    word = "";
  };

  //---

  auto skipWord = [&]() {
    parse.skipSpace();

    while (! parse.eof()) {
      if      (parse.isSpace())
        break;
      else if (parse.isChar(';'))
        break;
      else if (parse.isChar('|') || parse.isChar('&'))
        break;
      else if (parse.isChar('(') || parse.isChar(')'))
        break;
      else if (parse.isChar('<') || parse.isChar('>'))
        break;
      else if (parse.isChar('\\')) {
        word += parse.readChar();

        if (! parse.eof())
          word += parse.readChar();
      }
      else {
        word += parse.readChar();
      }
    }

    if (parse.getPos() > pos)
      type = TokenType::WORD;
  };

  //---

  while (! parse.eof()) {
    if      (parse.isChar('"')) {
      flushToken(/*skip*/false);

      type = TokenType::DQUOTE_STRING;

      int pos1 = parse.getPos();

      if (! parse.skipString()) {
        throw "Invalid string";
        break; // error
      }

      auto str = parse.getBefore(pos1);

      word += str.substr(1, str.size() - 2);

      flushToken();
    }
    else if (parse.isChar('\'')) {
      flushToken(/*skip*/false);

      type = TokenType::SQUOTE_STRING;

      int pos1 = parse.getPos();

      if (! parse.skipString()) {
        throw "Invalid string";
        break; // error
      }

      auto str = parse.getBefore(pos1);

      word += str.substr(1, str.size() - 2);

      flushToken();
    }
    else if (parse.isChar('`')) {
      flushToken(/*skip*/false);

      type = TokenType::BACKQUOTE_STRING;

      int pos1 = parse.getPos();

      if (! parse.skipString()) {
        throw "Invalid string";
        break; // error
      }

      auto str = parse.getBefore(pos1);

      word += str.substr(1, str.size() - 2);

      flushToken();
    }
    else if (parse.isChar('(') || parse.isChar(')')) {
      flushToken(/*skip*/false);

      type = TokenType::OPERATOR;

      subType = int(parse.isChar('(') ? Operator::OPEN_RBRACKETS :
                                        Operator::CLOSE_RBRACKETS);

      word += parse.readChar();

      flushToken();
    }
    // <, << or <<-, <&, <>
    else if (parse.isChar('<')) {
      flushToken(/*skip*/false);

      type = TokenType::OPERATOR;

      word += parse.readChar();

      if      (parse.isChar('<')) {
        word += parse.readChar();

        if (parse.isChar('-')) {
          word += parse.readChar();

          subType = int(Operator::LESS_LESS_MINUS);
        }
        else
          subType = int(Operator::LESS_LESS);
      }
      else if (parse.isChar('&')) {
        word += parse.readChar();

        subType = int(Operator::FILE_INPUT_ERR);
      }
      else if (parse.isChar('>')) {
        word += parse.readChar();

        subType = int(Operator::LESS_GREATER);
      }
      else
        subType = int(Operator::FILE_INPUT);

      flushToken();
    }
    // >, >>, >&, >|
    else if (parse.isChar('>')) {
      flushToken(/*skip*/false);

      type = TokenType::OPERATOR;

      word += parse.readChar();

      if      (parse.isChar('>')) {
        word += parse.readChar();

        subType = int(Operator::FILE_OUTPUT_APPEND);
      }
      else if (parse.isChar('&')) {
        word += parse.readChar();

        subType = int(Operator::FILE_OUTPUT_ERR);
      }
      else if (parse.isChar('|')) {
        word += parse.readChar();

        subType = int(Operator::FILE_OUTPUT_PIPE);
      }
      else
        subType = int(Operator::FILE_OUTPUT);

      flushToken();

      skipWord();

      flushToken();
    }
    // |, ||, |&
    else if (parse.isChar('|')) {
      flushToken(/*skip*/false);

      type = TokenType::OPERATOR;

      word += parse.readChar();

      if      (parse.isChar('|')) {
        word += parse.readChar();

        subType = int(Operator::OR);
      }
      else if (parse.isChar('&')) {
        word += parse.readChar();

        subType = int(Operator::PIPE_ERR);
      }
      else
        subType = int(Operator::PIPE);

      flushToken();
    }
    // &, &&, &>, &>>
    else if (parse.isChar('&')) {
      flushToken(/*skip*/false);

      type = TokenType::OPERATOR;

      word += parse.readChar();

      if      (parse.isChar('&')) {
        word += parse.readChar();

        subType = int(Operator::AND);
      }
      else if (parse.isChar('>')) {
        word += parse.readChar();

        if (parse.isChar('>')) {
          word += parse.readChar();

          subType = int(Operator::BG_GREATER_GREATER);
        }
        else
          subType = int(Operator::BG_GREATER);
      }
      else
        subType = int(Operator::BG);

      flushToken();
    }
    // ;, ;;, ;;&, ;&
    else if (parse.isChar(';')) {
      flushToken(/*skip*/false);

      type = TokenType::OPERATOR;

      word += parse.readChar();

      if      (parse.isChar(';')) {
        word += parse.readChar();

        if (parse.isChar('&')) {
          word += parse.readChar();

          subType = int(Operator::DOUBLE_SEMI_COLON_BG); // case end (test and continue)
        }
        else
          subType = int(Operator::DOUBLE_SEMI_COLON); // case end (continue - no test)
      }
      else if (parse.isChar('&')) {
        word += parse.readChar();

        subType = int(Operator::SEMI_COLON_BG); // case end
      }
      else
        subType = int(Operator::SEMI_COLON);

      flushToken();
    }
    else if (parse.isChar('$')) {
      if      (parse.isString("$'")) {
        flushToken(/*skip*/false);

        parse.skipChar();

        type = TokenType::SQUOTE_STRING;

        int pos1 = parse.getPos();

        if (! parse.skipString()) {
          throw "Invalid string";
          break; // error
        }

        auto str = parse.getBefore(pos1);

        auto str1 = str.substr(1, str.size() - 2);

        CStrParse parse1(str1);

        std::string str2;

        while (! parse1.eof()) {
          if (parse1.isChar('\\')) {
            parse1.skipChar();

            auto c = parse1.readChar();

            switch (c) {
              case 'a' : str2 += '\a'; break;
              case 'b' : str2 += '\b'; break;
              case 'e' : case 'E' : str2 += '\033'; break;
              case 'f' : str2 += '\f'; break;
              case 'n' : str2 += '\n'; break;
              case 'r' : str2 += '\r'; break;
              case 't' : str2 += '\t'; break;
              case 'v' : str2 += '\v'; break;
              case '\\': str2 += '\\'; break;
              case '\'': str2 += '\''; break;
              case '\"': str2 += '\"'; break;
              case '?' : str2 += '?'; break;

              // \nnn
              case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': {
                uint n = (c - '0');

                for (uint j = 0; j < 2; ++j) {
                  if (! parse1.eof() && CStrUtil::isodigit(parse1.getCharAt()))
                    n += 8*n + (parse1.readChar() - '0');
                }

                str2 += char(n);

                break;
              }
              // \xHH
              case 'x': {
                if (! parse1.eof()) {
                  uint n = 0;

                  uint hex_value;

                  for (uint j = 0; j < 2; ++j) {
                    if (! parse1.eof() && CStrUtil::isxdigit(parse1.getCharAt())) {
                      if (CStrUtil::decodeHexChar(parse1.readChar(), &hex_value))
                        n += 16*n + hex_value;
                    }
                  }

                  if (n < 255)
                    str2 += char(n);
                }
                else
                 str2 += c;

                break;
              }
              // \uHHHH
              case 'u': {
                if (! parse1.eof()) {
                  uint n = 0;

                  for (uint j = 0; j < 4; ++j) {
                    uint hex_value;

                    if (! parse1.eof() && CStrUtil::isxdigit(parse1.getCharAt())) {
                      if (CStrUtil::decodeHexChar(parse1.readChar(), &hex_value))
                        n += 16*n + hex_value;
                    }
                  }

                  if (n < 255)
                    str2 += char(n);
                }
                else
                  str2 += c;

                break;
              }
              // \UHHHHHHHH
              case 'U': {
                if (! parse1.eof()) {
                  uint n = 0;

                  for (uint j = 0; j < 8; ++j) {
                    uint hex_value;

                    if (! parse1.eof() && CStrUtil::isxdigit(parse1.getCharAt())) {
                      if (CStrUtil::decodeHexChar(parse1.readChar(), &hex_value))
                        n += 16*n + hex_value;
                    }
                  }

                  if (n < 255)
                    str2 += char(n);
                }
                else
                  str2 += c;

                break;
              }
              // \cx
              case 'c': {
                if (! parse1.eof()) {
                  int n = 0;

                  auto c1 = parse1.readChar();

                  if      (std::islower(c1))
                    n += c1 - 'a';
                  else if (std::isupper(c1))
                    n += c1 - 'A';
                }
                else
                  str2 += c;

                break;
              }
            }
          }
          else {
            str2 += parse1.readChar();
          }
        }

        word += str2;

        flushToken();
      }
      else {
        word += parse.readChar();

        if (parse.isChar('(')) {
          word += parse.readChar();

          int brackets = 1;

          while (! parse.eof()) {
            if      (parse.isChar('(')) {
              word += parse.readChar();

              ++brackets;
            }
            else if (parse.isChar(')')) {
              word += parse.readChar();

              if (brackets == 1)
                break;

              --brackets;
            }
            else
              word += parse.readChar();
          }
        }
      }
    }
    else if (parse.isChar('=')) {
      if (type == TokenType::WORD) {
        parse.skipChar();

        subType = int(Operator::ASSIGN);

        flushToken();
      }
      else
        word += parse.readChar();
    }
    else if (parse.isSpace()) {
      flushToken();

      type = TokenType::NONE;
    }
    else if (parse.isAlpha() || parse.isChar('_')) {
      if (type == TokenType::NONE) {
        while (parse.isAlnum() || parse.isChar('_'))
          word += parse.readChar();

        type = TokenType::WORD;
      }
      else {
        word += parse.readChar();
      }
    }
    else if (parse.isDigit()) {
      if (type == TokenType::NONE) {
        while (parse.isDigit())
          word += parse.readChar();

        type = TokenType::NUMBER;
      }
    }
    else if (parse.isChar('\\')) {
      parse.skipChar();

      if (! parse.eof()) {
        flushToken();

        word += parse.readChar();

        type = TokenType::SQUOTE_STRING;

        flushToken();
      }
    }
    else {
      word += parse.readChar();
    }
  }

  flushToken();

  return true;
}

bool
App::
tokensToCmds(const Tokens &tokens, Cmds &cmds) const
{
  auto *th = const_cast<App *>(this);

  int brackets = 0;

  CmdP cmd = std::make_shared<Cmd>(th);

  auto flushCmd = [&]() {
    if (cmd->name() != "") {
      cmds.push_back(cmd);

      if (isDebug())
        cmd->print(std::cerr);
    }

    cmd = std::make_shared<Cmd>(th);
  };

  size_t it = 0;
  auto   nt = tokens.size();

  while (it < nt) {
    const auto &token = tokens[it++];

    const std::string &str = token.str();

    if      (token.type() == TokenType::WORD && token.opType() == Operator::ASSIGN) {
      if (it >= nt)
        throw "missing value";

      const auto &token1 = tokens[it++];

      const std::string &value = token1.str();

      th->addVariable(str, value);
    }
    else if (token.type() == TokenType::OPERATOR) {
      auto subType = token.opType();

      if      (subType == Operator::OPEN_RBRACKETS) {
        ++brackets;
      }
      else if (subType == Operator::CLOSE_RBRACKETS) {
        if (brackets == 0) {
          throw "bad ()";
          break; // error
        }

        --brackets;
      }
      else if (subType == Operator::SEMI_COLON) {
        cmd->addOpt(token, "");

        if (brackets == 0)
          flushCmd();
      }
      else if (subType == Operator::FILE_INPUT || subType == Operator::FILE_INPUT_ERR ||
               subType == Operator::FILE_OUTPUT || subType == Operator::FILE_OUTPUT_ERR) {
        auto fileStr = (it < nt ? tokens[it++].str() : "");

        cmd->addOpt(token, fileStr);
      }
      else if (subType == Operator::OR || subType == Operator::AND) {
        cmd->addOpt(token, "");

        if (brackets == 0)
          flushCmd();
      }
      else {
        cmd->addOpt(token, "");

        if (subType == Operator::PIPE || subType == Operator::PIPE_ERR)
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

        if (expandToken(token, tokens1)) {
          for (const auto &token1 : tokens1)
            cmd->addArg(token1);
        }
        else
          cmd->addArg(token);
      }
    }
  }

  if (brackets != 0) {
    throw "bad ()";
    return false;
  }

  flushCmd();

  return true;
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

    if (! lineToTokens(value, tokens1))
      return false;

    for (const auto &token1 : tokens1)
      args.push_back(token1.str());
  }

  return true;
}

bool
App::
expandToken(const Token &token, Tokens &tokens) const
{
  bool changed = false;

  //---

  // no expansion in single quote string
  if (token.type() == TokenType::SQUOTE_STRING)
    return changed;

  //---

  const std::string &str = token.str();

  // 1) brace expansion
  CFileMatch fm;

  StringArray args1;

  if (fm.expandBraces(str, args1)) {
    changed = true;

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
    args1.clear();

    if (expandArg(token, args1)) {
      changed = true;

      for (const auto &arg1 : args1)
        tokens.push_back(arg1);
    }
    else
      tokens.push_back(token);
  }

  return changed;
}

bool
App::
expandArg(const Token &token, StringArray &args) const
{
  bool changed = false;

  //---

  // no expansion in single quote string
  if (token.type() == TokenType::SQUOTE_STRING)
    return changed;

  //---

  auto token1 = token;

  //---

  // 2) expand tilde (~)
  {
  auto str1 = token1.str();

  std::string arg2;

  if (CFile::expandTilde(str1, arg2)) {
    token1 = arg2;
    changed = true;
  }
  }

  //---

  // 3) expand variables
  {
  auto str1 = token1.str();

  if (token1.type() == TokenType::DQUOTE_STRING) {
    std::string str2;

    if (expandStringVariable(str1, str2)) {
      token1 = str2;

      changed = true;
    }
  }
  else {
    if (str1[0] == '$') {
      std::string arg2;

      if (expandVariableToken(token1, arg2)) {
        token1 = arg2;

        changed = true;
      }
    }
  }
  }

  //---

  // 4) command subsitution

  //---

  // 5) arithmetic expansion

  //---

  // 6) process subsitution

  //---

  // 7) word splitting

  //---

  // 8 filename expansion

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
expandVariableToken(const Token &token, std::string &arg1) const
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
  // $(<var>) , $((<var>))
  else if (arg[i] == '(') {
    ++i;

    // $((<var>))
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
    // $(<var>)
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
    if      (parse.isChar('\\')) {
      str1 += parse.readChar();

      if (! parse.eof())
        str1 += parse.readChar();
    }
    else if (parse.isChar('$')) {
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

    if (! getVariable(name, value))
      return "";

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
getLine(bool &complete) const
{
  auto line = readLine_->readLine();

  complete = (line.empty() || line[line.size() - 1] != '\\');

  if (! complete)
    line = line.substr(0, line.size() - 1);

  return line;
}

std::string
App::
getInputPrompt() const
{
  std::string prompt;

  if (complete_) {
    if (! getVariable("PS1", prompt))
      prompt = "$ ";
  }
  else {
    if (! getVariable("PS2", prompt))
      prompt = "> ";
  }

  auto prompt1 = adjustPrompt(prompt);

  return prompt1;
}

std::string
App::
adjustPrompt(const std::string &prompt) const
{
  std::string prompt1;

  CStrParse parse(prompt);

  int non_print = 0; // not counted to prompt width ?

  while (! parse.eof()) {
    if (parse.isChar('\\')) {
      parse.skipChar();

      if (! parse.eof()) {
        // date
        if      (parse.isChar('d')) {
          prompt1 += COSTime::getTimeString("%a %m %d");

          parse.skipChar();
        }
        // general date
        else if (parse.isString("D{")) {
          parse.skipChars(2);

          std::string tstr;

          while (! parse.eof() && ! parse.isChar('}'))
            tstr += parse.readChar();

          if (! parse.eof())
            parse.skipChar();

          prompt1 += COSTime::getTimeString(tstr);
        }
        // histname
        else if (parse.isChar('e')) {
          prompt1 += "";

          parse.skipChar();
        }
        // short hostname
        else if (parse.isChar('h')) {
          prompt1 += COSUser::getHostName(); // up to first '.'

          parse.skipChar();
        }
        // full hostname
        else if (parse.isChar('H')) {
          prompt1 += COSUser::getHostName();

          parse.skipChar();
        }
        // number of jobs
        else if (parse.isChar('j')) {
          // TODO

          parse.skipChar();
        }
        // basename of shell device
        else if (parse.isChar('l')) {
          // TODO

          parse.skipChar();
        }
        // newline
        else if (parse.isChar('n')) {
          prompt1 += "\n";

          parse.skipChar();
        }
        // carriage return
        else if (parse.isChar('r')) {
          prompt1 += "\r";

          parse.skipChar();
        }
        // shell (TODO: variable, executable base name)
        else if (parse.isChar('s')) {
          prompt1 += "bash";

          parse.skipChar();
        }
        // time (12 hr)
        else if (parse.isChar('t')) {
          prompt1 += COSTime::getTimeString("%H:%M:%S");

          parse.skipChar();
        }
        // time (24 hr)
        else if (parse.isChar('T')) {
          prompt1 += COSTime::getTimeString("%H:%M:%S");

          parse.skipChar();
        }
        // time (am/pm)
        else if (parse.isChar('@')) {
          prompt1 += COSTime::getTimeString("%H:%M:%S"); // TODO

          parse.skipChar();
        }
        // time (24 hr)
        else if (parse.isChar('A')) {
          prompt1 += COSTime::getTimeString("%H:%M");

          parse.skipChar();
        }
        // user name
        else if (parse.isChar('u')) {
          prompt1 += COSUser::getUserName();

          parse.skipChar();
        }
        // version (short)
        else if (parse.isChar('v')) {
          // TODO
          parse.skipChar();
        }
        // version (long)
        else if (parse.isChar('V')) {
          // TODO
          parse.skipChar();
        }
        // current working dir
        else if (parse.isChar('w')) {
          prompt1 += COSFile::getCurrentDir();

          parse.skipChar();
        }
        // basename of current working dir
        else if (parse.isChar('W')) {
          std::string dir, base;
          COSFile::splitPath(COSFile::getCurrentDir(), dir, base);
          prompt1 += base;

          parse.skipChar();
        }
        // history number
        else if (parse.isChar('!')) {
          prompt1 += std::to_string(history_->commandNum());

          parse.skipChar();
        }
        // command number
        else if (parse.isChar('#')) {
          prompt1 += std::to_string(commandNum()); // TODO

          parse.skipChar();
        }
        // uid char
        else if (parse.isChar('$')) {
          uint uid;
          COSUser::getUserId(&uid);
          prompt1 += (uid ? '$' : '#');

          parse.skipChar();
        }
        // octal char : \nnn
        else if (parse.isChar('\\')) {
          parse.skipChar();

          int n = 0;

          for (uint j = 0; j < 3; ++j) {
            if (! parse.eof() && CStrUtil::isodigit(parse.getCharAt()))
              n += 8*n + (parse.readChar() - '0');
          }

          prompt1 += char(n);
        }
        // backslash
        else if (parse.isChar('\\')) {
          prompt1 += "\\";

          parse.skipChar();
        }
        // \[ : start escaped text
        else if (parse.isChar('[')) {
          ++non_print;

          parse.skipChar();
        }
        // \] : end escaped text
        else if (parse.isChar(']')) {
          if (non_print)
            --non_print;

          parse.skipChar();
        }
        else {
          prompt1 += "//";
          prompt1 += parse.readChar();
        }
      }
      else
        prompt1 += "//";
    }
    else
      prompt1 += parse.readChar();
  }

  return prompt1;
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
      cmd->command()->addPipeDest(1);

      if (cmd->isPipeErr())
        cmd->command()->addPipeDest(2);

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
  if (cmd->isWait()) {
    if (cmd->command()->isState(CCommand::State::RUNNING))
      cmd->wait();
  }
  else {
     int n = 1; // TODO

     int pid = cmd->command()->getPid();

     std::cout << "[" << n << "] " << pid << "\n";

     bgCommands_.insert(cmd->commandp());
  }
}

void
App::
checkBgCommands()
{
  int count = 0;

  auto p1 = bgCommands_.begin();

  while (p1 != bgCommands_.end()) {
    auto cmd = *p1;

    auto state = cmd->getState();

    if (state == CCommand::State::RUNNING || state == CCommand::State::STOPPED ||
        state == CCommand::State::EXITED)
      count++;

    if (state == CCommand::State::EXITED) {
      int n = 1; // TODO

      std::cout << "[" << n << "]    Done                  ";

      std::cout << " " << cmd->getCommandString();

      std::cout << "\n";

      bgCommands_.erase(cmd);

      p1 = bgCommands_.begin();
    }
    else
      ++p1;
  }
}

//---

void
App::
addHistory(const std::string &line)
{
  history_->addCommand(line);
}

void
App::
listHistory() const
{
  int  num         = 0;
  bool showNumbers = false;
  bool showTime    = true;
  bool reverse     = false;

  history_->display(num, showNumbers, showTime, reverse);
}

//---

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

bool
App::
completeVariable(const std::string &name, std::string &expandedName) const
{
  CGlob glob(name + "*");

  glob.setAllowOr(false);
  glob.setAllowNonPrintable(true);

  StringArray names;

  for (const auto &pv : variableMap_) {
    if (glob.compare(pv.first))
      names.push_back(pv.first);
  }

  if (names.empty())
    return false;

  expandedName = CStrUtil::mostMatch(names);

  if (expandedName.size() <= name.size())
    return false;

  return true;
}

bool
App::
showMatchingVariables(const std::string &name)  const
{
  CGlob glob(name + "*");

  glob.setAllowOr(false);
  glob.setAllowNonPrintable(true);

  StringArray words;

  for (const auto &pv : variableMap_) {
    if (glob.compare(pv.first))
      words.push_back(pv.first);
  }

  if (words.empty())
    return false;

  CStrUtil::sort(words);

  StringArray uniqWords;

  CStrUtil::uniq(words, uniqWords);

  std::cout << "\n";

  printWords(uniqWords);

  return true;
}

void
App::
listVariables() const
{
  for (const auto &pv : variableMap_) {
    std::cout << pv.first << "=" << pv.second.value() << "\n";
  }
}

//---

void
App::
addSignalHandlers()
{
  COSSignal::addSignalHandler(SIGTERM,
    reinterpret_cast<COSSignal::SignalHandler>(genericHandler));
  COSSignal::addSignalHandler(SIGINT ,
    reinterpret_cast<COSSignal::SignalHandler>(interruptHandler));
  COSSignal::addSignalHandler(SIGQUIT,
    reinterpret_cast<COSSignal::SignalHandler>(genericHandler));
  COSSignal::addSignalHandler(SIGTTIN,
    reinterpret_cast<COSSignal::SignalHandler>(genericHandler));
  COSSignal::addSignalHandler(SIGTTOU,
    reinterpret_cast<COSSignal::SignalHandler>(genericHandler));
  COSSignal::addSignalHandler(SIGTSTP,
    reinterpret_cast<COSSignal::SignalHandler>(genericHandler));
  COSSignal::addSignalHandler(SIGHUP,
    reinterpret_cast<COSSignal::SignalHandler>(termHandler));
}

void
App::
genericHandler(int /*num*/)
{
#if 0
  std::cerr << "Signal " << signalName(num) << "(" << num << ") received\n";
#endif
}

void
App::
interruptHandler(int)
{
  // break out of any loops
  //std::cerr << "interruptHandler\n";
}

void
App::
termHandler(int)
{
  // resend SIGHUP to all jobs (SIGCONT then SIGHUP to stopped jobs)

  // quit
}

//---

void
App::
printWords(const StringArray &words) const
{
  int maxLen = CStrUtil::maxLen(words);

  int screenRows, screenCols;

  COSTerm::getCharSize(&screenRows, &screenCols);

  int numWords = int(words.size());

  int wordsPerLine = std::max(screenCols / (maxLen + 1), 1);

  int numLines = numWords / wordsPerLine;

  if ((numWords % wordsPerLine) != 0)
    ++numLines;

  int i = 0;
  int j = 0;

  while (i < numWords && j < numLines) {
    int len = int(words[i].size());

    std::cout << words[i];

    for (int k = 0; k <= maxLen - len; ++k)
      std::cout << " ";

    i += numLines;

    if (i >= numWords) {
      std::cout << "\n";

      ++j;

      i = j;
    }
  }
}

//---

bool
App::
isKeyword(const std::string &name, Keyword &keyword) const
{
  using KeywordMap = std::map<std::string, Keyword>;

  static KeywordMap keywordMap;

  if (keywordMap.empty()) {
    auto addKeyword =[&](const std::string &keyname, Keyword keytype) {
      keywordMap[keyname] = keytype;
    };

    addKeyword("if"  , Keyword::IF);
    addKeyword("then", Keyword::THEN);
    addKeyword("else", Keyword::ELSE);
    addKeyword("elif", Keyword::ELIF);
    addKeyword("fi"  , Keyword::FI);

    addKeyword("case", Keyword::CASE);
    addKeyword("esac", Keyword::ESAC);

    addKeyword("for"  , Keyword::FOR);
    addKeyword("while", Keyword::WHILE);
    addKeyword("until", Keyword::UNTIL);
    addKeyword("do"   , Keyword::DO); // if third word of for
    addKeyword("done" , Keyword::DONE);

    addKeyword("function", Keyword::FUNCTION);
    addKeyword("in"      , Keyword::IN); // if third word of case/select/for

    addKeyword("select", Keyword::SELECT);

    addKeyword("!" , Keyword::NOT);
    addKeyword("{" , Keyword::OPEN_BRACE);
    addKeyword("}" , Keyword::CLOSE_BRACE);
    addKeyword("[[", Keyword::OPEN_BLOCK);
    addKeyword("]]", Keyword::CLOSE_BLOCK);
  }

  auto p = keywordMap.find(name);

  if (p == keywordMap.end())
    return false;

  keyword = (*p).second;

  return true;
}

//---

void
App::
listAliases() const
{
  for (const auto &pa : aliases_) {
    const auto &alias = pa.second;

    std::cout << alias->name() << " =";

    for (const auto &value : alias->values())
      std::cout << " " << value;

    std::cout << "\n";
  }
}

void
App::
listAlias(const std::string &name) const
{
  auto pa = aliases_.find(name);
  if (pa == aliases_.end()) return;

  const auto &alias = (*pa).second;

  std::cout << alias->name() << " =";

  for (const auto &value : alias->values())
    std::cout << " " << value;

  std::cout << "\n";
}

void
App::
addAlias(const std::string &name, const StringArray &values)
{
  aliases_[name] = std::make_shared<Alias>(name, values);
}

//------

ReadLine::
ReadLine(App *app) :
 app_(app)
{
}

std::string
ReadLine::
readLine()
{
  auto prompt = app_->getInputPrompt();

  setPrompt(prompt);

  fflush(stdout);
  fflush(stderr);

  std::string line;

  try {
    line = CReadLine::readLine();

    if (eof()) {
      std::cout << "\n";

      //auto *variable = app_->lookupVariable("ignoreeof");

      //if (! variable)
      //  app_->setExit(true, 0);

      std::cerr << "Use \"exit\" to leave shell.\n";
    }
  }
#if 0
  catch (struct Err *err) {
    if (app_->isDebug())
      std::cerr << "[" << err->file << ":" << err->line << "] ";

    if (err->qualifier != "")
      std::cerr << err->qualifier << ": " << err->message << "\n";
    else
      std::cerr << err->message << "\n";
  }
#endif
  catch (...) {
    std::cerr << "Unhandled Exception thrown\n";
  }

  return line;
}

bool
ReadLine::
completeLine(const std::string &line, std::string &completedLine)
{
  Tokens tokens;

  try {
    if (! app_->lineToTokens(line, tokens))
      return false;
  }
  catch (...) {
  }

  if (tokens.empty())
    return false;

  const auto &token = tokens.back();

  const auto &str= token.str();

  if (! str.empty() && str[0] == '$') {
    auto str1 = str.substr(1);

    std::string str2;

    if (! app_->completeVariable(str1, str2))
      return false;

    completedLine = str2.substr(str1.size());
  }
  else {
    return false;
  }

  return true;
}

bool
ReadLine::
showComplete(const std::string &line)
{
  Tokens tokens;

  try {
    if (! app_->lineToTokens(line, tokens))
      return false;
  }
  catch (...) {
  }

  if (tokens.empty())
    return false;

  const auto &token = tokens.back();

  const auto &str= token.str();

  if (! str.empty() && str[0] == '$') {
    auto str1 = str.substr(1);

    app_->showMatchingVariables(str1);
  }
  else {
    return false;
  }

  return true;
}

bool
ReadLine::
getPrevCommand(std::string &line)
{
  auto *history = app_->history();

  if (! history->hasPrevCommand())
    return false;

  line = history->getPrevCommand();

  return true;
}

bool
ReadLine::
getNextCommand(std::string &line)
{
  auto *history = app_->history();

  if (! history->hasNextCommand())
    return false;

  line = history->getNextCommand();

  return true;
}

void
ReadLine::
beep()
{
#if 0
  auto *nobeep = app_->lookupVariable("nobeep");

  if (! nobeep)
    CReadLine::beep();
#else
  CReadLine::beep();
#endif
}

void
ReadLine::
interrupt()
{
  CReadLine::interrupt();
}

void
ReadLine::
timeout()
{
#if 0
  app_->readTimeout();
#endif
}

//------

History::
History(App *app) :
 app_(app)
{
}

void
History::
addCommand(const std::string &line)
{
  auto sline = CStrUtil::stripSpaces(line);
  if (sline == "") return;

  //setCurrent(sline);

  history_.addCommand(sline);

  //updateSize();

  commandNum_ = history_.getLastCommandNum() + 1;
}

bool
History::
hasPrevCommand()
{
  std::string command;

  if (commandNum_ <= 0 || ! history_.getCommand(commandNum_ - 1, command))
    return false;

  return true;
}

bool
History::
hasNextCommand()
{
  return true;
}

std::string
History::
getPrevCommand()
{
  std::string command;

  if      (  history_.getCommand(commandNum_ - 1, command))
    --commandNum_;
  else if (! history_.getCommand(commandNum_, command))
    command = "";

  return command;
}

std::string
History::
getNextCommand()
{
  std::string command;

  if (! history_.getCommand(commandNum_ + 1, command))
    return "";

  ++commandNum_;

  return command;
}

void
History::
display(int num, bool showNumbers, bool showTime, bool reverse)
{
  //updateSize();

  //if (num > getSize())
  //  num = -1;

  history_.display(num, showNumbers, showTime, reverse);
}

//------

void
AliasCommand::
exec(const Cmd *cmd)
{
  auto numArgs = cmd->numArgs();

  if (numArgs == 0) {
    cmd->app()->listAliases();
  }
  else {
    const auto &token = cmd->arg(0);

    const auto &arg = token.str();

    if (arg[arg.size() - 1] == '=') {
      auto name = arg.substr(0, arg.size() - 1);

      StringArray values;

      for (uint i = 1; i < numArgs; ++i) {
        const auto &token1 = cmd->arg(i);

        values.push_back(token1.str());
      }

      cmd->app()->addAlias(name, values);
    }
    else {
      for (uint i = 0; i < numArgs; ++i) {
        const auto &token1 = cmd->arg(i);

        cmd->app()->listAlias(token1.str());
      }
    }
  }
}

//---

void
CdCommand::
exec(const Cmd *cmd)
{
  auto numArgs = cmd->numArgs();

  if (numArgs == 0) {
    auto dirname = COSUser::getUserHome();

    app_->changeDir(dirname);
  }
  else {
    for (uint i = 0; i < numArgs; ++i) {
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
ExitCommand::
exec(const Cmd *)
{
  exit(0);
}

//---

void
HistoryCommand::
exec(const Cmd *)
{
  app_->listHistory();
}

//---

void
PrintfCommand::
exec(const Cmd *cmd)
{
  auto numArgs = cmd->numArgs();

  std::string format;
  StringArray args;
  bool        first = true;

  for (uint i = 0; i < numArgs; ++i) {
    const auto &token = cmd->arg(i);

    const auto &str = token.str();

    if (str == "-v") {
      // TODO
      ++i;
    }
    else {
      if (first) {
        format = str;

        first = false;
      }
      else
        args.push_back(str);
    }
  }

  if (format == "")
    return;

  format = CStrUtil::replaceEscapeCodes(format);

  class PrintF : public CPrintF {
   public:
    PrintF(const std::string &format, const StringArray &args) :
     CPrintF(format), args_(args) {
    }

    int         getInt     () const { if (! getArg()) return 0; return std::stoi(arg_); }
    long        getLong    () const { if (! getArg()) return 0; return std::stoi(arg_); }
    long        getLongLong() const { if (! getArg()) return 0; return std::stoi(arg_); }
    double      getDouble  () const { if (! getArg()) return 0; return std::stod(arg_); }
    std::string getString  () const { if (! getArg()) return 0; return arg_; }

  private:
    bool getArg() const {
      if (argNum_ >= args_.size()) return false;
      arg_ = args_[argNum_++];
      return true;
    }

   private:
    StringArray         args_;
    mutable uint        argNum_ { 0 };
    mutable std::string arg_;
  };

  PrintF printf(format, args);

  std::cout << printf.format();

  return;
}

//---

void
SetCommand::
exec(const Cmd *)
{
  app_->listVariables();
}

//------

Cmd::
Cmd(App *app) :
 app_(app)
{
}

void
Cmd::
addOpt(const Token &token, const std::string &arg)
{
  auto subType = token.opType();

  assert(subType != Operator::NONE);

  if      (subType == Operator::PIPE || subType == Operator::PIPE_ERR) {
    pipe_    = true;
    pipeErr_ = (subType == Operator::PIPE_ERR);
  }
  else if (subType == Operator::FILE_OUTPUT || subType == Operator::FILE_OUTPUT_ERR)
    output_ = arg;
  else if (subType == Operator::FILE_INPUT || subType == Operator::FILE_INPUT_ERR)
    input_ = arg;
  else if (subType == Operator::BG)
    wait_ = false;
  else if (subType == Operator::AND)
    isAnd_ = true;
  else if (subType == Operator::OR)
    isOr_ = true;

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
      CPathList pathList;

      pathList.addEnvValue("PATH");

      std::string path;

      if (! pathList.search(name_, path)) {
        std::cerr << name_ + ": Command not found." << "\n";
        return;
      }

      StringArray args;

      for (const auto &arg : args_)
        args.push_back(arg.str());

      th->command_ = std::make_unique<CCommand>(name_, path, args);
    }
  }
}

void
Cmd::
start() const
{
  bool stop = false;

  if      (isAnd()) {
    if (app_->returnCode() != 0)
      stop = true;
  }
  else if (isOr()) {
    if (app_->returnCode() == 0)
      stop = true;
  }

  if      (builtin_) {
    if (! stop)
      builtin_->exec(this);
  }
  else if (command_) {
    try {
      if (! stop)
        command_->start();
      else
        command_->stop();
    }
    catch (const std::string message) {
      std::cerr << message << "\n";
    }
  }
}

void
Cmd::
wait() const
{
  if (command_) {
    command_->wait();

    app_->setReturnCode(command_->getReturnCode());
  }
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
