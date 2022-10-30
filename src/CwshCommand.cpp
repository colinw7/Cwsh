#include <CwshI.h>

CwshCommandData::
CwshCommandData(Cwsh *cwsh, const std::vector<std::string> &words) :
 cwsh_(cwsh)
{
  type_ = CwshCommandUtil::getType(cwsh, words);

  if (type_ == CwshCommandType::UNIX) {
    std::string name = CStrUtil::removeEscapeChars(words[0]);

    std::vector<std::string> args;

    uint num_words = uint(words.size());

    for (uint i = 1; i < num_words; ++i)
      args.push_back(CStrUtil::removeEscapeChars(words[i]));

    try {
      std::string path = CwshUnixCommand::search(cwsh, name);

      command_ = new CwshCommand(cwsh_, name, path, args);
    }
    catch (struct CwshErr *error) {
      bool found = false;

      if (num_words == 1 && CFile::exists(name) && CFile::isDirectory(name)) {
        args.clear(); args.push_back(name);

        CwshShellCommand *shell_command = cwsh_->lookupShellCommand("cd");

        // TODO: lookup and expand any cd alias
        command_ = new CwshCommand(cwsh_, "cd", CwshShellCommandMgr::runProc,
                                   static_cast<CCommand::CallbackData>(shell_command), args);

        found = true;
      }
      else if (num_words == 1 && CFile::exists(name) && CFile::isRegular(name)) {
        std::string::size_type pos = name.rfind('.');

        if (pos != std::string::npos) {
          std::string suffix = name.substr(pos + 1);

          CwshAutoExec *exec = cwsh->lookupAutoExec(suffix);

          if (exec) {
            std::string              cmd;
            std::vector<std::string> eargs;

            if (exec->substitute(name, cmd, eargs)) {
              std::string path = CwshUnixCommand::search(cwsh, cmd);

              command_ = new CwshCommand(cwsh_, cmd, path, eargs);

              found = true;
            }
          }
        }
      }

      if (! found)
        CWSH_THROWQ(name, "Command not found.");
    }
  }
  else if (type_ == CwshCommandType::SHELL) {
    std::string name = CStrUtil::removeEscapeChars(words[0]);

    std::vector<std::string> cargs;

    uint num_words = uint(words.size());

    for (uint i = 1; i < num_words; ++i)
      cargs.push_back(CStrUtil::removeEscapeChars(words[i]));

    CwshShellCommand *shell_command = cwsh_->lookupShellCommand(name);

    command_ = new CwshCommand(cwsh_, name, CwshShellCommandMgr::runProc,
                               static_cast<CCommand::CallbackData>(shell_command), cargs);
  }
  else if (type_ == CwshCommandType::FUNCTION) {
    std::string name = CStrUtil::removeEscapeChars(words[0]);

    std::vector<std::string> fargs;

    uint num_words = int(words.size());

    for (uint i = 1; i < num_words; ++i)
      fargs.push_back(CStrUtil::removeEscapeChars(words[i]));

    CwshFunction *function = cwsh_->lookupFunction(name);

    command_ = new CwshCommand(cwsh_, name, CwshFunction::runProc,
                               static_cast<CCommand::CallbackData>(function), fargs);
  }
  else if (type_ == CwshCommandType::LABEL)
    ;
  else if (type_ == CwshCommandType::SUBSHELL) {
    std::string name = "";

    std::string line = CStrUtil::toString(words, 1, uint(words.size()) - 2);

    std::vector<std::string> sargs;

    sargs.push_back(line);

    command_ = new CwshCommand(cwsh_, name, CwshCommandUtil::processLineProc, cwsh_, sargs, true);
  }
  else if (type_ == CwshCommandType::PROCESS) {
    if (words.size() > 1)
      CWSH_THROW("Too many arguments.");

    CwshProcess *process = cwsh_->getActiveProcess(words[0]);

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

CwshCommandData::
~CwshCommandData()
{
}

//------------

CwshCommand::
CwshCommand(Cwsh *cwsh, const std::string &name, const std::string &path,
            const StringVectorT &args, bool do_fork) :
 CCommand(name, path, args, do_fork), cwsh_(cwsh)
{
}

CwshCommand::
CwshCommand(Cwsh *cwsh, const std::string &name, CallbackProc proc, CallbackData data,
            const StringVectorT &args, bool do_fork) :
 CCommand(name, proc, data, args, do_fork), cwsh_(cwsh)
{
}

CwshCommand::
~CwshCommand()
{
}

void
CwshCommand::
setState(State state)
{
  if (getState() == state) return;

  CCommand::setState(state);

  if (notify_) {
    CwshProcess *process = cwsh_->lookupProcess(getPid());
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
CwshCommandUtil::
parseCommandLines(Cwsh *cwsh, const std::string &str, CwshCmdLineArray &cmds)
{
  // Split String Into Words

  CwshWordArray words;

  CwshWord::toWords(str, words);

  if (cwsh->getDebug()) {
    std::cerr << "Split String Into Words\n";

    CwshWord::printWords(words);
  }

  return CwshCmdSplit::wordsToCommandLines(words, cmds);
}

bool
CwshCommandUtil::
parseCommandGroups(Cwsh *cwsh, const std::string &str, CwshCmdGroupArray &groups)
{
  // Split String Into Words

  CwshWordArray words;

  CwshWord::toWords(str, words);

  if (cwsh->getDebug()) {
    std::cerr << "Split String Into Words\n";

    CwshWord::printWords(words);
  }

  //------

  // Split Words Into Commands

  CwshCmdArray cmds;

  CwshCmdSplit::wordsToCommands(words, cmds);

  if (cwsh->getDebug()) {
    std::cerr << "Split Words Into Commands\n";

    CwshCmd::displayCmdArray(cmds);
  }

  //------

  // Replace Aliased Commands

  CwshCmdArray cmds1;

  uint num_cmds = uint(cmds.size());

  for (uint i = 0; i < num_cmds; ++i) {
    CwshCmdArray alias_cmds;

    if (cwsh->substituteAlias(cmds[i], alias_cmds)) {
      delete cmds[i];

      copy(alias_cmds.begin(), alias_cmds.end(), back_inserter(cmds1));
    }
    else
      cmds1.push_back(cmds[i]);
  }

  if (cwsh->getDebug()) {
    std::cerr << "Replace Aliased Commands\n";

    CwshCmd::displayCmdArray(cmds1);
  }

  //------

  groupCommands(cmds1, groups);

  //------

  return true;
}

bool
CwshCommandUtil::
groupCommands(CwshCmdArray cmds, CwshCmdGroupArray &groups)
{
  CwshCmdArray cmds1;

  uint num_cmds = uint(cmds.size());

  for (uint i = 0; i < num_cmds; ++i) {
    cmds1.push_back(cmds[i]);

    if (cmds[i]->getSeparator().getType() == CwshCmdSeparatorType::NORMAL) {
      if (cmds1.size() > 0) {
        auto *group = new CwshCmdGroup(cmds1);

        groups.push_back(group);

        cmds1.clear();
      }
    }
  }

  if (cmds1.size() > 0) {
    auto *group = new CwshCmdGroup(cmds1);

    groups.push_back(group);
  }

  return true;
}

CwshCmdArray
CwshCommandUtil::
parseCommandGroup(Cwsh *cwsh, CwshCmdGroup *group)
{
  const CwshCmdArray cmds = group->getCommands();

  //------

  // Replace Variables

  uint num_cmds = uint(cmds.size());

  for (uint i = 0; i < num_cmds; ++i) {
    CwshShellCommand *shell_command = cwsh->lookupShellCommand(cmds[i]->getWord(0).getWord());

    if (shell_command && shell_command->getNoExpand())
      continue;

    //------

    CwshWordArray words;

    int num_words = cmds[i]->getNumWords();

    for (int j = 0; j < num_words; ++j) {
      CwshWordArray words1;

      const CwshWord &word = cmds[i]->getWord(j);

      CwshVariableParser vparser(cwsh, word);

      if (vparser.expandVariables(words1))
        copy(words1.begin(), words1.end(), back_inserter(words));
      else
        words.push_back(word);
    }

    cmds[i]->setWords(words);
  }

  if (cwsh->getDebug()) {
    std::cerr << "Replace Variables\n";

    CwshCmd::displayCmdArray(cmds);
  }

  //------

  // Replace backquotes

  num_cmds = uint(cmds.size());

  for (uint i = 0; i < num_cmds; ++i) {
    CwshShellCommand *shell_command = cwsh->lookupShellCommand(cmds[i]->getWord(0).getWord());

    if (shell_command && shell_command->getNoExpand())
      continue;

    //------

    CwshWordArray cmd_words;
    bool          cmd_words_changed = false;

    int num_words = cmds[i]->getNumWords();

    for (int j = 0; j < num_words; ++j) {
      const CwshWord &word = cmds[i]->getWord(j);

      CwshWordArray in_place_words;

      CwshInPlaceCommand icmd(cwsh, word);

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

    CwshCmd::displayCmdArray(cmds);
  }

  //------

  // Expand Tildes

  num_cmds = uint(cmds.size());

  for (uint i = 0; i < num_cmds; ++i) {
    CwshShellCommand *shell_command = cwsh->lookupShellCommand(cmds[i]->getWord(0).getWord());

    if (shell_command && shell_command->getNoWildcards())
      continue;

    //------

    int num_words = cmds[i]->getNumWords();

    for (int j = 0; j < num_words; ++j) {
      std::string str;

      if (CFile::expandTilde(cmds[i]->getWord(j).getWord(), str))
        cmds[i]->setWord(j, CwshWord(str));
    }
  }

  if (cwsh->getDebug()) {
    std::cerr << "Expand Tildes\n";

    CwshCmd::displayCmdArray(cmds);
  }

  //------

  // Expand Braces

  num_cmds = uint(cmds.size());

  for (uint i = 0; i < num_cmds; ++i) {
    CwshShellCommand *shell_command = cwsh->lookupShellCommand(cmds[i]->getWord(0).getWord());

    if (shell_command && shell_command->getNoWildcards())
      continue;

    //------

    CwshWordArray words;

    int num_words = cmds[i]->getNumWords();

    for (int j = 0; j < num_words; ++j) {
      const CwshWord &word = cmds[i]->getWord(j);

      CwshWordArray words1;

      if (CwshBraces::expand(word, words1))
        copy(words1.begin(), words1.end(), back_inserter(words));
      else
        words.push_back(word);
    }

    cmds[i]->setWords(words);
  }

  if (cwsh->getDebug()) {
    std::cerr << "Expand Braces\n";

    CwshCmd::displayCmdArray(cmds);
  }

  //------

  // Expand Wildcards

  num_cmds = uint(cmds.size());

  for (uint i = 0; i < num_cmds; ++i) {
    CwshShellCommand *shell_command = cwsh->lookupShellCommand(cmds[i]->getWord(0).getWord());

    if (shell_command && shell_command->getNoWildcards())
      continue;

    //------

    CwshWordArray words;

    int num_words = cmds[i]->getNumWords();

    for (int j = 0; j < num_words; ++j) {
      const CwshWord &word = cmds[i]->getWord(j);

      CwshWordArray words1;

      CwshPattern pattern(cwsh);

      if (pattern.expandWordToFiles(word, words1))
        copy(words1.begin(), words1.end(), back_inserter(words));
      else
        words.push_back(word);
    }

    cmds[i]->setWords(words);
  }

  if (cwsh->getDebug()) {
    std::cerr << "Expand Wildcards\n";

    CwshCmd::displayCmdArray(cmds);
  }

  //------

  // Remove Quotes

  num_cmds = uint(cmds.size());

  for (uint i = 0; i < num_cmds; ++i) {
    CwshShellCommand *shell_command = cwsh->lookupShellCommand(cmds[i]->getWord(0).getWord());

    if (shell_command && shell_command->getNoExpand())
      continue;

    //------

    int num_words = cmds[i]->getNumWords();

    for (int j = 0; j < num_words; ++j) {
      CwshWord word = cmds[i]->getWord(j);

      word.removeQuotes();

      cmds[i]->setWord(j, word);
    }
  }

  if (cwsh->getDebug()) {
    std::cerr << "Remove Quotes\n";

    CwshCmd::displayCmdArray(cmds);
  }

  //------

  // Get Command Redirection

  num_cmds = uint(cmds.size());

  for (uint i = 0; i < num_cmds; ++i) {
    CwshShellCommand *shell_command = cwsh->lookupShellCommand(cmds[i]->getWord(0).getWord());

    if (shell_command && shell_command->getNoExpand())
      continue;

    //------

    CwshWordArray words;

    int num_words = cmds[i]->getNumWords();

    for (int j = 0; j < num_words; ++j) {
      const std::string &word = cmds[i]->getWord(j).getWord();

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

CwshCommandType
CwshCommandUtil::
getType(Cwsh *cwsh, const std::vector<std::string> &words)
{
  const std::string &name = words[0];

  if (name.size() > 0 && name[0] == '(')
    return CwshCommandType::SUBSHELL;

  //------

  if (name.size() > 0 && name[0] == '%')
    return CwshCommandType::PROCESS;

  //------

  if (words.size() == 2 && words[1] == ":")
    return CwshCommandType::LABEL;

  //------

  CwshFunction *function = cwsh->lookupFunction(name);

  if (function)
    return CwshCommandType::FUNCTION;

  //------

  CwshShellCommand *command = cwsh->lookupShellCommand(name);

  if (command)
    return CwshCommandType::SHELL;

  //------

  return CwshCommandType::UNIX;
}

void
CwshCommandUtil::
processLineProc(const std::vector<std::string> &args, CCommand::CallbackData data)
{
  auto *cwsh = reinterpret_cast<Cwsh *>(data);

  cwsh->processInputLine(args[0]);
}
