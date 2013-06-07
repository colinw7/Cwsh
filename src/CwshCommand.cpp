#include "CwshI.h"

CwshCommandData::
CwshCommandData(Cwsh *cwsh, const vector<string> &words) :
 cwsh_(cwsh)
{
  type_ = CwshCommandUtil::getType(cwsh, words);

  if (type_ == CWSH_COMMAND_TYPE_UNIX) {
    string name = CStrUtil::removeEscapeChars(words[0]);

    vector<string> args;

    uint num_words = words.size();

    for (uint i = 1; i < num_words; ++i)
      args.push_back(CStrUtil::removeEscapeChars(words[i]));

    try {
      string path = CwshUnixCommand::search(cwsh, name);

      command_ = new CwshCommand(cwsh_, name, path, args);
    }
    catch (struct CwshErr *error) {
      bool found = false;

      if (num_words == 1 && CFile::exists(name) && CFile::isDirectory(name)) {
        args.clear(); args.push_back(name);

        CwshShellCommand *shell_command = cwsh_->lookupShellCommand("cd");

        // TODO: lookup and expand any cd alias
        command_ = new CwshCommand(cwsh_, "cd", CwshShellCommandMgr::runProc,
                                   (CCommand::CallbackData) shell_command, args);

        found = true;
      }
      else if (num_words == 1 && CFile::exists(name) && CFile::isRegular(name)) {
        string::size_type pos = name.rfind('.');

        if (pos != string::npos) {
          string suffix = name.substr(pos + 1);

          CwshAutoExec *exec = cwsh->lookupAutoExec(suffix);

          if (exec != NULL) {
            string         cmd;
            vector<string> args;

            if (exec->substitute(name, cmd, args)) {
              string path = CwshUnixCommand::search(cwsh, cmd);

              command_ = new CwshCommand(cwsh_, cmd, path, args);

              found = true;
            }
          }
        }
      }

      if (! found)
        CWSH_THROWQ(name, "Command not found.");
    }
  }
  else if (type_ == CWSH_COMMAND_TYPE_SHELL) {
    string name = CStrUtil::removeEscapeChars(words[0]);

    vector<string> args;

    uint num_words = words.size();

    for (uint i = 1; i < num_words; ++i)
      args.push_back(CStrUtil::removeEscapeChars(words[i]));

    CwshShellCommand *shell_command = cwsh_->lookupShellCommand(name);

    command_ = new CwshCommand(cwsh_, name, CwshShellCommandMgr::runProc,
                               (CCommand::CallbackData) shell_command, args);
  }
  else if (type_ == CWSH_COMMAND_TYPE_FUNCTION) {
    string name = CStrUtil::removeEscapeChars(words[0]);

    vector<string> args;

    uint num_words = words.size();

    for (uint i = 1; i < num_words; ++i)
      args.push_back(CStrUtil::removeEscapeChars(words[i]));

    CwshFunction *function = cwsh_->lookupFunction(name);

    command_ = new CwshCommand(cwsh_, name, CwshFunction::runProc,
                               (CCommand::CallbackData) function, args);
  }
  else if (type_ == CWSH_COMMAND_TYPE_LABEL)
    ;
  else if (type_ == CWSH_COMMAND_TYPE_SUBSHELL) {
    string name = "";

    string line = CStrUtil::toString(words, 1, words.size() - 2);

    vector<string> args;

    args.push_back(line);

    command_ = new CwshCommand(cwsh_, name, CwshCommandUtil::processLineProc,
                               cwsh_, args, true);
  }
  else if (type_ == CWSH_COMMAND_TYPE_PROCESS) {
    if (words.size() > 1)
      CWSH_THROW("Too many arguments.");

    CwshProcess *process = cwsh_->getActiveProcess(words[0]);

    if (process == NULL)
      CWSH_THROW("No such job.");

    cout << process->getCommandString() << endl;

    process->resume();

    process->wait();
  }
  else {
    string name = CStrUtil::removeEscapeChars(words[0]);

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
 CCommand(name, path, args, do_fork), cwsh_(cwsh), notify_(false), stateChanged_(false)
{
}

CwshCommand::
CwshCommand(Cwsh *cwsh, const std::string &name, CallbackProc proc, CallbackData data,
            const StringVectorT &args, bool do_fork) :
 CCommand(name, proc, data, args, do_fork), cwsh_(cwsh), notify_(false), stateChanged_(false)
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

    cout << "[" << process->getNum() << "] ";

    cout << getPid() << " ";

    if      (getState() == CCommand::STOPPED_STATE)
      cout << "Suspended             ";
    else if (getState() == CCommand::EXITED_STATE)
      cout << "Exited                ";
    else if (getState() == CCommand::SIGNALLED_STATE)
      cout << "Signalled             ";
    else if (getState() == CCommand::RUNNING_STATE)
      cout << "Running               ";
    else
      cout << "????                  ";

    process->print();

    cout << endl;
  }
  else
    stateChanged_ = true;
}

//------------

bool
CwshCommandUtil::
parseCommandLines(Cwsh *cwsh, const string &str, CwshCmdLineArray &cmds)
{
  // Split String Into Words

  CwshWordArray words;

  CwshWord::toWords(str, words);

  if (cwsh->getDebug()) {
    cerr << "Split String Into Words" << endl;

    CwshWord::printWords(words);
  }

  return CwshCmdSplit::wordsToCommandLines(words, cmds);
}

bool
CwshCommandUtil::
parseCommandGroups(Cwsh *cwsh, const string &str, CwshCmdGroupArray &groups)
{
  // Split String Into Words

  CwshWordArray words;

  CwshWord::toWords(str, words);

  if (cwsh->getDebug()) {
    cerr << "Split String Into Words" << endl;

    CwshWord::printWords(words);
  }

  //------

  // Split Words Into Commands

  CwshCmdArray cmds;

  CwshCmdSplit::wordsToCommands(words, cmds);

  if (cwsh->getDebug()) {
    cerr << "Split Words Into Commands" << endl;

    CwshCmd::displayCmdArray(cmds);
  }

  //------

  // Replace Aliased Commands

  CwshCmdArray cmds1;

  uint num_cmds = cmds.size();

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
    cerr << "Replace Aliased Commands" << endl;

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

  uint num_cmds = cmds.size();

  for (uint i = 0; i < num_cmds; ++i) {
    cmds1.push_back(cmds[i]);

    if (cmds[i]->getSeparator().getType() == CWSH_COMMAND_SEPARATOR_NORMAL) {
      if (cmds1.size() > 0) {
        CwshCmdGroup *group = new CwshCmdGroup(cmds1);

        groups.push_back(group);

        cmds1.clear();
      }
    }
  }

  if (cmds1.size() > 0) {
    CwshCmdGroup *group = new CwshCmdGroup(cmds1);

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

  uint num_cmds = cmds.size();

  for (uint i = 0; i < num_cmds; ++i) {
    CwshShellCommand *shell_command = cwsh->lookupShellCommand(cmds[i]->getWord(0).getWord());

    if (shell_command != NULL && shell_command->getNoExpand())
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
    cerr << "Replace Variables" << endl;

    CwshCmd::displayCmdArray(cmds);
  }

  //------

  // Replace backquotes

  num_cmds = cmds.size();

  for (uint i = 0; i < num_cmds; ++i) {
    CwshShellCommand *shell_command = cwsh->lookupShellCommand(cmds[i]->getWord(0).getWord());

    if (shell_command != NULL && shell_command->getNoExpand())
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
        uint num_in_place_words = in_place_words.size();

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
    cerr << "Replace backquotes" << endl;

    CwshCmd::displayCmdArray(cmds);
  }

  //------

  // Expand Tildes

  num_cmds = cmds.size();

  for (uint i = 0; i < num_cmds; ++i) {
    CwshShellCommand *shell_command = cwsh->lookupShellCommand(cmds[i]->getWord(0).getWord());

    if (shell_command != NULL && shell_command->getNoWildcards())
      continue;

    //------

    int num_words = cmds[i]->getNumWords();

    for (int j = 0; j < num_words; ++j) {
      string str;

      if (CFile::expandTilde(cmds[i]->getWord(j).getWord(), str))
        cmds[i]->setWord(j, CwshWord(str));
    }
  }

  if (cwsh->getDebug()) {
    cerr << "Expand Tildes" << endl;

    CwshCmd::displayCmdArray(cmds);
  }

  //------

  // Expand Braces

  num_cmds = cmds.size();

  for (uint i = 0; i < num_cmds; ++i) {
    CwshShellCommand *shell_command = cwsh->lookupShellCommand(cmds[i]->getWord(0).getWord());

    if (shell_command != NULL && shell_command->getNoWildcards())
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
    cerr << "Expand Braces" << endl;

    CwshCmd::displayCmdArray(cmds);
  }

  //------

  // Expand Wildcards

  num_cmds = cmds.size();

  for (uint i = 0; i < num_cmds; ++i) {
    CwshShellCommand *shell_command = cwsh->lookupShellCommand(cmds[i]->getWord(0).getWord());

    if (shell_command != NULL && shell_command->getNoWildcards())
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
    cerr << "Expand Wildcards" << endl;

    CwshCmd::displayCmdArray(cmds);
  }

  //------

  // Remove Quotes

  num_cmds = cmds.size();

  for (uint i = 0; i < num_cmds; ++i) {
    CwshShellCommand *shell_command = cwsh->lookupShellCommand(cmds[i]->getWord(0).getWord());

    if (shell_command != NULL && shell_command->getNoExpand())
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
    cerr << "Remove Quotes" << endl;

    CwshCmd::displayCmdArray(cmds);
  }

  //------

  // Get Command Redirection

  num_cmds = cmds.size();

  for (uint i = 0; i < num_cmds; ++i) {
    CwshShellCommand *shell_command = cwsh->lookupShellCommand(cmds[i]->getWord(0).getWord());

    if (shell_command != NULL && shell_command->getNoExpand())
      continue;

    //------

    CwshWordArray words;

    int num_words = cmds[i]->getNumWords();

    for (int j = 0; j < num_words; ++j) {
      const string &word = cmds[i]->getWord(j).getWord();

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
getType(Cwsh *cwsh, const vector<string> &words)
{
  const string &name = words[0];

  if (name.size() > 0 && name[0] == '(')
    return CWSH_COMMAND_TYPE_SUBSHELL;

  //------

  if (name.size() > 0 && name[0] == '%')
    return CWSH_COMMAND_TYPE_PROCESS;

  //------

  if (words.size() == 2 && words[1] == ":")
    return CWSH_COMMAND_TYPE_LABEL;

  //------

  CwshFunction *function = cwsh->lookupFunction(name);

  if (function != NULL)
    return CWSH_COMMAND_TYPE_FUNCTION;

  //------

  CwshShellCommand *command = cwsh->lookupShellCommand(name);

  if (command != NULL)
    return CWSH_COMMAND_TYPE_SHELL;

  //------

  return CWSH_COMMAND_TYPE_UNIX;
}

void
CwshCommandUtil::
processLineProc(const vector<string> &args, CCommand::CallbackData data)
{
  Cwsh *cwsh = (Cwsh *) data;

  cwsh->processInputLine(args[0]);
}
