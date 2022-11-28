#include <CwshI.h>

namespace Cwsh {

CommandData::
CommandData(App *cwsh, const std::vector<std::string> &words) :
 cwsh_(cwsh)
{
  type_ = CommandUtil::getType(cwsh, words);

  if (type_ == CommandType::UNIX) {
    auto name = CStrUtil::removeEscapeChars(words[0]);

    std::vector<std::string> args;

    auto num_words = words.size();

    for (uint i = 1; i < num_words; ++i)
      args.push_back(CStrUtil::removeEscapeChars(words[i]));

    try {
      auto path = UnixCommand::search(cwsh, name);

      command_ = std::make_shared<Command>(cwsh_, name, path, args);
    }
    catch (struct Err *error) {
      bool found = false;

      if (num_words == 1 && CFile::exists(name) && CFile::isDirectory(name)) {
        args.clear(); args.push_back(name);

        auto *shell_command = cwsh_->lookupShellCommand("cd");

        // TODO: lookup and expand any cd alias
        command_ = std::make_shared<Command>(cwsh_, "cd", ShellCommandMgr::runProc,
                     static_cast<CCommand::CallbackData>(shell_command), args);

        found = true;
      }
      else if (num_words == 1 && CFile::exists(name) && CFile::isRegular(name)) {
        auto pos = name.rfind('.');

        if (pos != std::string::npos) {
          auto suffix = name.substr(pos + 1);

          auto *exec = cwsh->lookupAutoExec(suffix);

          if (exec) {
            std::string              cmd;
            std::vector<std::string> eargs;

            if (exec->substitute(name, cmd, eargs)) {
              auto path = UnixCommand::search(cwsh, cmd);

              command_ = std::make_shared<Command>(cwsh_, cmd, path, eargs);

              found = true;
            }
          }
        }
      }

      if (! found)
        CWSH_THROWQ(name, "Command not found.");
    }
  }
  else if (type_ == CommandType::SHELL) {
    auto name = CStrUtil::removeEscapeChars(words[0]);

    std::vector<std::string> cargs;

    auto num_words = words.size();

    for (uint i = 1; i < num_words; ++i)
      cargs.push_back(CStrUtil::removeEscapeChars(words[i]));

    auto *shell_command = cwsh_->lookupShellCommand(name);

    command_ = std::make_shared<Command>(cwsh_, name, ShellCommandMgr::runProc,
                           static_cast<CCommand::CallbackData>(shell_command), cargs);
  }
  else if (type_ == CommandType::FUNCTION) {
    auto name = CStrUtil::removeEscapeChars(words[0]);

    std::vector<std::string> fargs;

    auto num_words = words.size();

    for (uint i = 1; i < num_words; ++i)
      fargs.push_back(CStrUtil::removeEscapeChars(words[i]));

    auto *function = cwsh_->lookupFunction(name);

    command_ = std::make_shared<Command>(cwsh_, name, Function::runProc,
                 static_cast<CCommand::CallbackData>(function), fargs);
  }
  else if (type_ == CommandType::LABEL)
    ;
  else if (type_ == CommandType::SUBSHELL) {
    std::string name = "";

    auto line = CStrUtil::toString(words, 1, uint(words.size()) - 2);

    std::vector<std::string> sargs;

    sargs.push_back(line);

    command_ = std::make_shared<Command>(cwsh_, name,
                 CommandUtil::processLineProc, cwsh_, sargs, true);
  }
  else if (type_ == CommandType::PROCESS) {
    if (words.size() > 1)
      CWSH_THROW("Too many arguments.");

    auto *process = cwsh_->getActiveProcess(words[0]);

    if (! process)
      CWSH_THROW("No such job.");

    std::cout << process->getCommandString() << "\n";

    process->resume();

    process->wait();
  }
  else {
    std::string name = CStrUtil::removeEscapeChars(words[0]);

    CWSH_THROW(name + ": Unimplemented Command.");
  }
}

CommandData::
~CommandData()
{
}

Command *
CommandData::
getCommand() const
{
  return command_.get();
}

//------------

Command::
Command(App *cwsh, const std::string &name, const std::string &path,
        const StringVectorT &args, bool doFork) :
 CCommand(name, path, args, doFork), cwsh_(cwsh)
{
}

Command::
Command(App *cwsh, const std::string &name, CallbackProc proc, CallbackData data,
        const StringVectorT &args, bool doFork) :
 CCommand(name, proc, data, args, doFork), cwsh_(cwsh)
{
}

Command::
~Command()
{
}

void
Command::
setState(State state)
{
  if (getState() == state) return;

  CCommand::setState(state);

  if (notify_) {
    auto *process = cwsh_->lookupProcess(getPid());
    if (! process) return;

    std::cout << "[" << process->getNum() << "] ";

    std::cout << getPid() << " ";

    if      (getState() == CCommand::State::STOPPED)
      std::cout << "Suspended             ";
    else if (getState() == CCommand::State::EXITED)
      std::cout << "Exited                ";
    else if (getState() == CCommand::State::SIGNALLED)
      std::cout << "Signalled             ";
    else if (getState() == CCommand::State::RUNNING)
      std::cout << "Running               ";
    else
      std::cout << "????                  ";

    process->print();

    std::cout << "\n";
  }
  else
    stateChanged_ = true;
}

//------------

bool
CommandUtil::
parseCommandLines(App *cwsh, const std::string &str, CmdLineArray &cmds)
{
  // Split String Into Words

  WordArray words;

  Word::toWords(str, words);

  if (cwsh->getDebug()) {
    std::cerr << "Split String Into Words\n";

    Word::printWords(words);
  }

  return CmdSplit::wordsToCommandLines(words, cmds);
}

bool
CommandUtil::
parseCommandGroups(App *cwsh, const std::string &str, CmdGroupArray &groups)
{
  // Split String Into Words

  WordArray words;

  Word::toWords(str, words);

  if (cwsh->getDebug()) {
    std::cerr << "Split String Into Words\n";

    Word::printWords(words);
  }

  //------

  // Split Words Into Commands

  CmdArray cmds;

  CmdSplit::wordsToCommands(words, cmds);

  if (cwsh->getDebug()) {
    std::cerr << "Split Words Into Commands\n";

    Cmd::displayCmdArray(cmds);
  }

  //------

  // Replace Aliased Commands

  CmdArray cmds1;

  auto num_cmds = cmds.size();

  for (uint i = 0; i < num_cmds; ++i) {
    CmdArray alias_cmds;

    if (cwsh->substituteAlias(cmds[i], alias_cmds)) {
      delete cmds[i];

      copy(alias_cmds.begin(), alias_cmds.end(), back_inserter(cmds1));
    }
    else
      cmds1.push_back(cmds[i]);
  }

  if (cwsh->getDebug()) {
    std::cerr << "Replace Aliased Commands\n";

    Cmd::displayCmdArray(cmds1);
  }

  //------

  groupCommands(cmds1, groups);

  //------

  return true;
}

bool
CommandUtil::
groupCommands(CmdArray cmds, CmdGroupArray &groups)
{
  CmdArray cmds1;

  auto num_cmds = cmds.size();

  for (uint i = 0; i < num_cmds; ++i) {
    cmds1.push_back(cmds[i]);

    if (cmds[i]->getSeparator().getType() == CmdSeparatorType::NORMAL) {
      if (cmds1.size() > 0) {
        auto group = std::make_shared<CmdGroup>(cmds1);

        groups.push_back(group);

        cmds1.clear();
      }
    }
  }

  if (cmds1.size() > 0) {
    auto group = std::make_shared<CmdGroup>(cmds1);

    groups.push_back(group);
  }

  return true;
}

CmdArray
CommandUtil::
parseCommandGroup(App *cwsh, CmdGroup *group)
{
  const auto &cmds = group->getCommands();

  //------

  // Replace Variables

  auto num_cmds = cmds.size();

  for (uint i = 0; i < num_cmds; ++i) {
    auto *shell_command = cwsh->lookupShellCommand(cmds[i]->getWord(0).getWord());

    if (shell_command && shell_command->getNoExpand())
      continue;

    //------

    WordArray words;

    int num_words = cmds[i]->getNumWords();

    for (int j = 0; j < num_words; ++j) {
      WordArray words1;

      const auto &word = cmds[i]->getWord(j);

      VariableParser vparser(cwsh, word);

      if (vparser.expandVariables(words1))
        copy(words1.begin(), words1.end(), back_inserter(words));
      else
        words.push_back(word);
    }

    cmds[i]->setWords(words);
  }

  if (cwsh->getDebug()) {
    std::cerr << "Replace Variables\n";

    Cmd::displayCmdArray(cmds);
  }

  //------

  // Replace backquotes

  num_cmds = cmds.size();

  for (uint i = 0; i < num_cmds; ++i) {
    auto *shell_command = cwsh->lookupShellCommand(cmds[i]->getWord(0).getWord());

    if (shell_command && shell_command->getNoExpand())
      continue;

    //------

    WordArray cmd_words;
    bool      cmd_words_changed = false;

    int num_words = cmds[i]->getNumWords();

    for (int j = 0; j < num_words; ++j) {
      const auto &word = cmds[i]->getWord(j);

      WordArray in_place_words;

      InPlaceCommand icmd(cwsh, word);

      if (icmd.expand(in_place_words)) {
        uint num_in_place_words = uint(in_place_words.size());

        for (uint k = 0; k < num_in_place_words; ++k)
          cmd_words.push_back(in_place_words[k]);

        cmd_words_changed = true;
      }
      else
        cmd_words.push_back(word);
    }

    if (cmd_words_changed)
      cmds[i]->setWords(cmd_words);
  }

  if (cwsh->getDebug()) {
    std::cerr << "Replace backquotes\n";

    Cmd::displayCmdArray(cmds);
  }

  //------

  // Expand Tildes

  num_cmds = cmds.size();

  for (uint i = 0; i < num_cmds; ++i) {
    auto *shell_command = cwsh->lookupShellCommand(cmds[i]->getWord(0).getWord());

    if (shell_command && shell_command->getNoWildcards())
      continue;

    //------

    int num_words = cmds[i]->getNumWords();

    for (int j = 0; j < num_words; ++j) {
      std::string str;

      if (CFile::expandTilde(cmds[i]->getWord(j).getWord(), str))
        cmds[i]->setWord(j, Word(str));
    }
  }

  if (cwsh->getDebug()) {
    std::cerr << "Expand Tildes\n";

    Cmd::displayCmdArray(cmds);
  }

  //------

  // Expand Braces

  num_cmds = cmds.size();

  for (uint i = 0; i < num_cmds; ++i) {
    auto *shell_command = cwsh->lookupShellCommand(cmds[i]->getWord(0).getWord());

    if (shell_command && shell_command->getNoWildcards())
      continue;

    //------

    WordArray words;

    int num_words = cmds[i]->getNumWords();

    for (int j = 0; j < num_words; ++j) {
      const auto &word = cmds[i]->getWord(j);

      WordArray words1;

      if (Braces::expand(word, words1))
        copy(words1.begin(), words1.end(), back_inserter(words));
      else
        words.push_back(word);
    }

    cmds[i]->setWords(words);
  }

  if (cwsh->getDebug()) {
    std::cerr << "Expand Braces\n";

    Cmd::displayCmdArray(cmds);
  }

  //------

  // Expand Wildcards

  num_cmds = cmds.size();

  for (uint i = 0; i < num_cmds; ++i) {
    auto *shell_command = cwsh->lookupShellCommand(cmds[i]->getWord(0).getWord());

    if (shell_command && shell_command->getNoWildcards())
      continue;

    //------

    WordArray words;

    int num_words = cmds[i]->getNumWords();

    for (int j = 0; j < num_words; ++j) {
      const auto &word = cmds[i]->getWord(j);

      WordArray words1;

      Pattern pattern(cwsh);

      if (pattern.expandWordToFiles(word, words1))
        copy(words1.begin(), words1.end(), back_inserter(words));
      else
        words.push_back(word);
    }

    cmds[i]->setWords(words);
  }

  if (cwsh->getDebug()) {
    std::cerr << "Expand Wildcards\n";

    Cmd::displayCmdArray(cmds);
  }

  //------

  // Remove Quotes

  num_cmds = cmds.size();

  for (uint i = 0; i < num_cmds; ++i) {
    auto *shell_command = cwsh->lookupShellCommand(cmds[i]->getWord(0).getWord());

    if (shell_command && shell_command->getNoExpand())
      continue;

    //------

    int num_words = cmds[i]->getNumWords();

    for (int j = 0; j < num_words; ++j) {
      auto word = cmds[i]->getWord(j);

      word.removeQuotes();

      cmds[i]->setWord(j, word);
    }
  }

  if (cwsh->getDebug()) {
    std::cerr << "Remove Quotes\n";

    Cmd::displayCmdArray(cmds);
  }

  //------

  // Get Command Redirection

  num_cmds = cmds.size();

  for (uint i = 0; i < num_cmds; ++i) {
    auto *shell_command = cwsh->lookupShellCommand(cmds[i]->getWord(0).getWord());

    if (shell_command && shell_command->getNoExpand())
      continue;

    //------

    WordArray words;

    int num_words = cmds[i]->getNumWords();

    for (int j = 0; j < num_words; ++j) {
      const auto &word = cmds[i]->getWord(j).getWord();

      if      (word == ">"  || word == ">!" ||
               word == ">>" || word == ">>!") {
        ++j;

        if (j >= cmds[i]->getNumWords())
          CWSH_THROW("Missing name for redirect.");

        cmds[i]->setStdOutFile(cmds[i]->getWord(j).getWord());

        if (word == ">!" || word == ">>!")
          cmds[i]->setStdOutClobber(true);
        else
          cmds[i]->setStdOutClobber(false);

        if (word == ">>" || word == ">>!")
          cmds[i]->setStdOutAppend(true);
        else
          cmds[i]->setStdOutAppend(false);
      }
      else if (word == ">&"  || word == ">&!" ||
               word == ">>&" || word == ">>&!") {
        ++j;

        if (j >= cmds[i]->getNumWords())
          CWSH_THROW("Missing name for redirect.");

        cmds[i]->setStdErrFile(cmds[i]->getWord(j).getWord());

        if (word == ">&!" || word == ">>&!")
          cmds[i]->setStdErrClobber(true);
        else
          cmds[i]->setStdErrClobber(false);

        if (word == ">>&" || word == ">>&!")
          cmds[i]->setStdErrAppend(true);
        else
          cmds[i]->setStdErrAppend(false);
      }
      else if (word == "<") {
        ++j;

        if (j >= cmds[i]->getNumWords())
          CWSH_THROW("Missing name for redirect.");

        cmds[i]->setStdInFile(cmds[i]->getWord(j).getWord());
      }
      else if (word == "<<") {
        ++j;

        if (j >= cmds[i]->getNumWords())
          CWSH_THROW("Missing name for redirect.");

        cmds[i]->setStdInToken(cmds[i]->getWord(j).getWord());
      }
      else
        words.push_back(cmds[i]->getWord(j));
    }

    cmds[i]->setWords(words);
  }

  return cmds;
}

CommandType
CommandUtil::
getType(App *cwsh, const std::vector<std::string> &words)
{
  const auto &name = words[0];

  if (name.size() > 0 && name[0] == '(')
    return CommandType::SUBSHELL;

  //------

  if (name.size() > 0 && name[0] == '%')
    return CommandType::PROCESS;

  //------

  if (words.size() == 2 && words[1] == ":")
    return CommandType::LABEL;

  //------

  auto *function = cwsh->lookupFunction(name);

  if (function)
    return CommandType::FUNCTION;

  //------

  auto *command = cwsh->lookupShellCommand(name);

  if (command)
    return CommandType::SHELL;

  //------

  return CommandType::UNIX;
}

void
CommandUtil::
processLineProc(const std::vector<std::string> &args, CCommand::CallbackData data)
{
  auto *cwsh = reinterpret_cast<App *>(data);

  cwsh->processInputLine(args[0]);
}

}
