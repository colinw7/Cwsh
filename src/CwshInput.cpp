#include <CwshI.h>
#include <CwshHistoryParser.h>
#include <CRGBName.h>
#include <CEscape.h>
#include <cstdio>
#include <unistd.h>

CwshInput::
CwshInput(Cwsh *cwsh) :
 cwsh_(cwsh)
{
  history_active_  = false;
  input_file_      = NULL;
  current_command_ = NULL;
}

CwshInput::
~CwshInput()
{
}

void
CwshInput::
execute(const string &filename)
{
  CFile file(filename);

  execute(&file);
}

void
CwshInput::
execute(CFile *file)
{
  bool save_history_active = history_active_;

  CFile *save_file = input_file_;

  input_file_ = file;

  if (! file->isStdIn())
    executeFile();
  else
    executeStdIn();

  input_file_ = save_file;

  history_active_ = save_history_active;
}

void
CwshInput::
executeFile()
{
  history_active_ = false;

  CwshLineArray lines;

  string line, line1;

  while (input_file_->readLine(line)) {
    uint len = line.size();

    if (len == 0)
      continue;

    if (line[len - 1] == '\\') {
      if (line1 != "")
        line1 += " " + line.substr(0, len - 1);
      else
        line1 = line.substr(0, len - 1);

      continue;
    }

    if (line1 != "")
      line = line1 + " " + line;

    lines.push_back(line);

    line1 = "";
  }

  if (line1 != "")
    lines.push_back(line1);

  if (lines.empty())
    return;

  executeLines(lines);
}

void
CwshInput::
executeStdIn()
{
  history_active_ = true;

  executeLines(true);
}

void
CwshInput::
executeLine(string &line)
{
  vector<string> lines;

  lines.push_back(line);

  executeLines(lines);
}

void
CwshInput::
executeLines(vector<string> &lines)
{
  cwsh_->startBlock(CWSH_BLOCK_TYPE_FILE, lines);

  executeLines(false);

  cwsh_->endBlock();
}

void
CwshInput::
executeLines(bool interactive)
{
  string line;

  while (! eof()) {
    line = getLine();

    if (interactive && cwsh_->getSilentMode()) {
      printf("%s", CEscape::commandToEscape(line, COSFile::getCurrentDir(), true).c_str());
      fflush(stdout);
    }

    processLine(line);

    if (interactive && cwsh_->getSilentMode()) {
      printf("%s", CEscape::commandToEscape(line, COSFile::getCurrentDir(), false).c_str());
      fflush(stdout);
    }

    if (cwsh_->getExit())
      break;
  }
}

bool
CwshInput::
eof()
{
  if (cwsh_->inBlock())
    return cwsh_->blockEof();

  return false;
}

void
CwshInput::
getBlock(CwshShellCommand *shell_command, CwshLineArray &lines)
{
  CwshPromptType prompt_type    = cwsh_->getPromptType();
  string         prompt_command = cwsh_->getPromptCommand();

  cwsh_->setPromptType   (CWSH_PROMPT_TYPE_EXTRA);
  cwsh_->setPromptCommand(shell_command->getName());

  while (! eof()) {
    string line = getLine();

    vector<string> words;

    CwshString::addWords(line, words);

    if (words.size() > 0) {
      string name = words[0];

      if (name == shell_command->getEndName())
        break;

      lines.push_back(line);

      CwshShellCommand *shell_command1 = cwsh_->lookupShellCommand(name);

      if (shell_command1 != NULL && shell_command1->isBlockCommand()) {
        CwshLineArray lines1;

        getBlock(shell_command1, lines1);

        copy(lines1.begin(), lines1.end(), back_inserter(lines));

        lines.push_back(shell_command1->getEndName());
      }
    }
    else
      lines.push_back(line);
  }

  cwsh_->setPromptType   (prompt_type);
  cwsh_->setPromptCommand(prompt_command);
}

void
CwshInput::
skipBlock(const string &line)
{
  vector<string> words;

  CwshString::addWords(line, words);

  if (words.size() == 0)
    return;

  CwshShellCommand *shell_command = cwsh_->lookupShellCommand(words[0]);

  if (shell_command == NULL || ! shell_command->isBlockCommand())
    return;

  CwshLineArray lines;

  getBlock(shell_command, lines);
}

string
CwshInput::
getLine()
{
  if (cwsh_->inBlock())
    return cwsh_->blockReadLine();

  string line;

  if (input_file_->isStdIn()) {
    line = cwsh_->readLine();

    if (line.size() > 0 && line[line.size() - 1] == '\\') {
      CwshPromptType prompt_type    = cwsh_->getPromptType();
      string         prompt_command = cwsh_->getPromptCommand();

      cwsh_->setPromptType   (CWSH_PROMPT_TYPE_EXTRA);
      cwsh_->setPromptCommand("");

      while (line.size() > 0 && line[line.size() - 1] == '\\') {
        string line1 = cwsh_->readLine();

        line = line.substr(0, line.size() - 1) + "\n" + line1;
      }

      cwsh_->setPromptType   (prompt_type);
      cwsh_->setPromptCommand(prompt_command);
    }

    cwsh_->displayExitedProcesses();
  }
  else {
    line = CwshString::readLineFromFile(input_file_);

    while (line.size() > 0 && line[line.size() - 1] == '\\') {
      string line1 = CwshString::readLineFromFile(input_file_);

      line = line.substr(0, line.size() - 1) + "\n" + line1;
    }
  }

  return line;
}

void
CwshInput::
processLine(const string &line)
{
  try {
    string line1;

    //------

    uint i = 0;

    CStrUtil::skipSpace(line, &i);

    uint len = line.size();

    if (i < len && line[i] != '#')
      line1 = line;

    //------

    // History Substitution

    bool output = false;

    CwshHistoryParser parser(cwsh_);

    string line2 = parser.parseLine(line1);

    if (line2 != line1) {
      output = true;

      line1 = line2;
    }

    //------

    // Remove Extra Spaces

    vector<string> words1;

    CwshString::addWords(line1, words1);

    line1 = CStrUtil::toString(words1, " ");

    //------

    // Output Line if Verbose

    CwshVariable *variable = cwsh_->lookupVariable("verbose");

    if (variable != NULL)
      output = true;

    if (output)
      std::cout << line1 << std::endl;

    //------

    // Set Current History Command

    cwsh_->setHistoryCurrent(line1);

    //------

    // Ignore if Blank Line or Comment

    if (line1 == "")
      return;

    //------

    // Add Command To History

    if (history_active_)
      cwsh_->addHistoryCommand(line1);

    //------

    // Convert Line to Command Lines

    CwshCmdGroupArray groups;

    CwshCommandUtil::parseCommandGroups(cwsh_, line1, groups);

    //------

    uint num_groups = groups.size();

    for (uint i = 0; i < num_groups; i++) {
      // Parse Command

      CwshCmdArray cmds = CwshCommandUtil::parseCommandGroup(cwsh_, groups[i]);

      // Execute Commands

      executeCommands(cmds);
    }

    //------

    std::for_each(groups.begin(), groups.end(), CDeletePointer());
  }
  catch (CwshHistoryIgnore i) {
    ;
  }
  catch (struct CwshErr *cthrow) {
    string qualifier = cthrow->qualifier;

    if (qualifier == "" && current_command_ != NULL)
      qualifier = current_command_->getCommand()->getName();

    if (cwsh_->getDebug())
      std::cerr << "[" << cthrow->file << ":" << cthrow->line << "] ";

    if (qualifier != "")
      std::cerr << qualifier << ": " << cthrow->message << std::endl;
    else
      std::cerr << cthrow->message << std::endl;
  }
  catch (...) {
    std::cerr << "Unhandled Exception thrown" << std::endl;
  }
}

void
CwshInput::
executeCommands(const CwshCmdArray &cmds)
{
  vector<CwshCommandData *>  pcommands;
  vector<string>             delete_files;
  CwshCommandData           *first_command = NULL;

  int num_cmds = cmds.size();

  for (int i = 0; i < num_cmds; i++) {
    if (cwsh_->getDebug()) {
      std::cerr << "Execute Command: ";

      cmds[i]->display();
    }

    //-----

    // Get Command

    vector<string> words;

    int num_words = cmds[i]->getNumWords();

    for (int j = 0; j < num_words; j++)
      words.push_back(cmds[i]->getWord(j).getWord());

    CwshCommandData *command = new CwshCommandData(cwsh_, words);

    CwshCommand *ccommand = command->getCommand();

    if (ccommand == NULL) {
      delete command;
      continue;
    }

    current_command_ = command;

    //-----

    // Echo Command

    CwshVariable *variable = cwsh_->lookupVariable("echo");

    if (variable != NULL)
      std::cout << ccommand->getCommandString() << std::endl;

    //-----

    // Set Redirection

    if      (cmds[i]->hasStdInToken()) {
      const string &filename = readStdInToken(cmds[i]->getStdInToken());

      ccommand->addFileSrc(filename);

      delete_files.push_back(filename);
    }
    else if (cmds[i]->hasStdInFile())
      ccommand->addFileSrc(cmds[i]->getStdInFile());

    if (cmds[i]->hasStdOutFile()) {
      ccommand->addFileDest(cmds[i]->getStdOutFile(), 1);

      CwshVariable *variable = cwsh_->lookupVariable("noclobber");

      if (cmds[i]->getStdOutClobber() || variable == NULL)
        ccommand->setFileDestOverwrite(true, 1);
      else
        ccommand->setFileDestOverwrite(false, 1);

      if (cmds[i]->getStdOutAppend())
        ccommand->setFileDestAppend(true, 1);
    }

    if (cmds[i]->hasStdErrFile()) {
      if (! cmds[i]->hasStdOutFile()) {
        ccommand->addFileDest(cmds[i]->getStdErrFile(), 1);

        CwshVariable *variable = cwsh_->lookupVariable("noclobber");

        if (cmds[i]->getStdErrClobber() || variable == NULL)
          ccommand->setFileDestOverwrite(true, 1);
        else
          ccommand->setFileDestOverwrite(false, 1);

        if (cmds[i]->getStdErrAppend())
          ccommand->setFileDestAppend(true, 1);
      }

      ccommand->addFileDest(cmds[i]->getStdErrFile(), 2);

      CwshVariable *variable = cwsh_->lookupVariable("noclobber");

      if (cmds[i]->getStdErrClobber() || variable == NULL)
        ccommand->setFileDestOverwrite(true, 2);
      else
        ccommand->setFileDestOverwrite(false, 2);

      if (cmds[i]->getStdErrAppend())
        ccommand->setFileDestAppend(true, 2);
    }

    // Run Command

    CwshCmdSeparatorType separator = cmds[i]->getSeparator().getType();

    if (separator != CWSH_COMMAND_SEPARATOR_PIPE &&
        separator != CWSH_COMMAND_SEPARATOR_PIPE_ERR) {
      int num_pcommands = pcommands.size();

      if (num_pcommands > 0) {
        ccommand->addPipeSrc();

        for (int k = 0; k < num_pcommands - 1; k++) {
          CwshCommand *pccommand = pcommands[k]->getCommand();

          if (first_command == NULL) {
            first_command = pcommands[k];

            pccommand->setProcessGroupLeader();
          }
          else
            pccommand->setProcessGroup(first_command->getCommand());

          pccommand->start();
        }

        CwshCommand *pccommand = pcommands[num_pcommands - 1]->getCommand();

        if (first_command == NULL) {
          first_command = pcommands[num_pcommands - 1];

          pccommand->setProcessGroupLeader();
        }
        else
          pccommand->setProcessGroup(first_command->getCommand());

        pccommand->start();

        ccommand->setProcessGroup(first_command->getCommand());

        ccommand->start();

        //------

        CwshProcess *process = cwsh_->addProcess(first_command);

        for (int k = 0; k < num_pcommands; k++)
          if (pcommands[k] != first_command)
            process->addSubCommand(pcommands[k]);

        if (first_command != command)
          process->addSubCommand(command);

        //------

        if (separator != CWSH_COMMAND_SEPARATOR_BACKGROUND) {
          for (int k = 0; k < num_pcommands; k++) {
            CwshCommand *pccommand = pcommands[k]->getCommand();

            pccommand->wait();
          }

          ccommand->wait();

          int status = ccommand->getReturnCode();

          cwsh_->defineVariable("status", status);

          std::for_each(pcommands.begin(), pcommands.end(), CDeletePointer());

          cwsh_->removeProcess(process);

          current_command_ = NULL;

          if (separator == CWSH_COMMAND_SEPARATOR_AND && status != 0)
            break;

          if (separator == CWSH_COMMAND_SEPARATOR_OR && status == 0)
            break;
        }
        else {
          std::cout << "[" << process->getNum() << "]";

          for (int k = 0; k < num_pcommands; k++) {
            CwshCommand *pccommand = pcommands[k]->getCommand();

            std::cout << " " << pccommand->getPid();
          }

          std::cout << " " << ccommand->getPid() << std::endl;
        }

        pcommands.clear();
      }
      else {
        if (ccommand->getDoFork()) {
          if (first_command == NULL) {
            first_command = command;

            ccommand->setProcessGroupLeader();
          }
          else
            ccommand->setProcessGroup(first_command->getCommand());
        }

        ccommand->start();

        CwshProcess *process = cwsh_->addProcess(first_command);

        if (separator != CWSH_COMMAND_SEPARATOR_BACKGROUND) {
          ccommand->wait();

          if (ccommand->getState() == CCommand::EXITED_STATE) {
            int status = ccommand->getReturnCode();

            cwsh_->defineVariable("status", status);

            cwsh_->removeProcess(process);

            current_command_ = NULL;

            if (separator == CWSH_COMMAND_SEPARATOR_AND && status != 0)
              break;

            if (separator == CWSH_COMMAND_SEPARATOR_OR && status == 0)
              break;
          }
          else {
            int pid = ccommand->getPid();

            std::cout << "[" << process->getNum() << "] " << pid << std::endl;
          }
        }
        else {
          int pid = ccommand->getPid();

          std::cout << "[" << process->getNum() << "] " << pid << std::endl;
        }
      }
    }
    else {
      if (pcommands.size() > 0)
        ccommand->addPipeSrc();

      ccommand->addPipeDest(1);

      if (separator == CWSH_COMMAND_SEPARATOR_PIPE_ERR)
        ccommand->addPipeDest(2);

      pcommands.push_back(command);
    }
  }

  int num_delete_files = delete_files.size();

  for (int i = 0; i < num_delete_files; i++)
    unlink(delete_files[i].c_str());
}

string
CwshInput::
readStdInToken(const string &token)
{
  CTempFile temp_file;

  cwsh_->setPromptType(CWSH_PROMPT_TYPE_EXTRA);

  while (1) {
    string line = cwsh_->readLine();

    if (line == token)
      break;

    if (token[0] != '"') {
      string line1 = processStdInLine(line);

      temp_file.getFile()->write(line1 + "\n");
    }
    else
      temp_file.getFile()->write(line + "\n");
  }

  temp_file.getFile()->close();

  cwsh_->setPromptType(CWSH_PROMPT_TYPE_NORMAL);

  return temp_file.getFile()->getPath();
}

string
CwshInput::
processStdInLine(const string &line)
{
  CwshWordArray words;

  CwshWord::toWords(line, words);

  if (cwsh_->getDebug()) {
    std::cerr << "Std In Line to Words" << std::endl;

    CwshWord::printWords(words);
  }

  int num_words = words.size();

  //------

  // Replace Variables

  CwshWordArray variable_words;

  for (int i = 0; i < num_words; i++) {
    CwshWordArray words2;

    CwshVariableParser vparser(cwsh_, words[i]);

    if (vparser.expandVariables(words2))
      copy(words2.begin(), words2.end(), back_inserter(variable_words));
    else
      variable_words.push_back(words[i]);
  }

  int num_variable_words = variable_words.size();

  //------

  // Replace backquotes

  CwshWordArray cmd_words;

  for (int i = 0; i < num_variable_words; i++) {
    const CwshWord &word = variable_words[i];

    CwshWordArray in_place_words;

    CwshInPlaceCommand icmd(cwsh_, word);

    if (icmd.expand(in_place_words)) {
      int num_in_place_words = in_place_words.size();

      for (int k = 0; k < num_in_place_words; ++k)
        cmd_words.push_back(in_place_words[k]);
    }
    else
      cmd_words.push_back(word);
  }

  //------

  return CwshWord::toString(cmd_words);
}

string
CwshInput::
processExprLine(const string &line)
{
  CwshWordArray words;

  CwshWord::toWords(line, words);

  if (cwsh_->getDebug()) {
    std::cerr << "Expr Line to Words" << std::endl;

    CwshWord::printWords(words);
  }

  int num_words = words.size();

  //------

  // Replace Variables

  CwshWordArray variable_words;

  for (int i = 0; i < num_words; i++) {
    CwshWordArray twords;

    CwshVariableParser vparser(cwsh_, words[i]);

    if (vparser.expandVariables(twords))
      copy(twords.begin(), twords.end(), back_inserter(variable_words));
    else
      variable_words.push_back(words[i]);
  }

  int num_variable_words = variable_words.size();

  //------

  // Replace backquotes

  CwshWordArray cmd_words;

  for (int i = 0; i < num_variable_words; i++) {
    const CwshWord &word = variable_words[i];

    CwshWordArray twords;

    CwshInPlaceCommand icmd(cwsh_, variable_words[i]);

    if (icmd.expand(twords))
      copy(twords.begin(), twords.end(), back_inserter(cmd_words));
    else
      cmd_words.push_back(word);
  }

  int num_cmd_words = cmd_words.size();

  //------

  // Expand Tildes

  for (int i = 0; i < num_cmd_words; i++) {
    string str;

    if (CFile::expandTilde(cmd_words[i].getWord(), str))
      cmd_words[i] = CwshWord(str);
  }

  //------

  // Expand Braces

  CwshWordArray brace_words;

  for (int i = 0; i < num_cmd_words; i++) {
    const CwshWord &word = cmd_words[i];

    CwshWordArray twords;

    if (CwshBraces::expand(word, twords))
      copy(twords.begin(), twords.end(), back_inserter(brace_words));
    else
      brace_words.push_back(word);
  }

  int num_brace_words = brace_words.size();

  //------

  // Expand Wildcards

  CwshWordArray wildcard_words;

  for (int i = 0; i < num_brace_words; i++) {
    const CwshWord &word = brace_words[i];

    CwshWordArray twords;

    CwshPattern pattern(cwsh_);

    if (pattern.expandWordToFiles(word, twords))
      copy(twords.begin(), twords.end(), back_inserter(wildcard_words));
    else
      wildcard_words.push_back(word);
  }

  //------

  return CwshWord::toString(wildcard_words);
}

string
CwshInput::
getPrompt()
{
  CwshVariable *prompt_var;

  if (cwsh_->getPromptType() == CWSH_PROMPT_TYPE_NORMAL)
    prompt_var = cwsh_->lookupVariable("prompt");
  else
    prompt_var = cwsh_->lookupVariable("prompt1");

  CwshVariable *color_var = cwsh_->lookupVariable("prompt_color");

  string prompt_string;

  if (cwsh_->getPromptType() != CWSH_PROMPT_TYPE_NORMAL)
    prompt_string += cwsh_->getPromptCommand();

  if (prompt_var)
    prompt_string += prompt_var->getValue(0);
  else {
    if (cwsh_->getPromptType() == CWSH_PROMPT_TYPE_NORMAL)
      prompt_string += "> ";
    else
      prompt_string += "? ";
  }

  if (color_var) {
    double r, g, b;

    if (CRGBName::lookup(color_var->getValue(0), &r, &g, &b)) {
      int fg = (b >= 0.5 ? 4 : 0) | (g >= 0.5 ? 2 : 0) | (r >= 0.5 ? 1 : 0);

      prompt_string = CStrUtil::strprintf("[%dm", 30 + fg) + prompt_string + "[0m";
    }
  }

  return prompt_string;
}
