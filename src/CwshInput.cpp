#include <CwshI.h>
#include <CwshHistoryParser.h>
#include <CRGBName.h>
#include <CEscapeColors.h>
#include <CEscape.h>
#include <CStrParse.h>
#include <unistd.h>

namespace Cwsh {

Input::
Input(App *cwsh) :
 cwsh_(cwsh)
{
}

Input::
~Input()
{
}

bool
Input::
execute(const std::string &filename)
{
  CFile file(filename);

  if (! file.exists())
    return false;

  return executeFile(&file);
}

bool
Input::
execute(CFile *file)
{
  return executeFile(file);
}

bool
Input::
executeFile(CFile *file)
{
  bool rc = true;

  bool   saveHistoryActive = historyActive_;
  CFile* saveFile          = inputFile_;

  historyActive_ = false;
  inputFile_     = file;

  if (! file->isStdIn()) {
    if (! executeCurrentFile())
      rc = false;
  }
  else {
    if (! executeStdIn())
      rc = false;
  }

  inputFile_     = saveFile;
  historyActive_ = saveHistoryActive;

  return rc;
}

bool
Input::
executeCurrentFile()
{
  LineArray lines;

  std::string line, line1;

  int lineNum = 1;

  while (inputFile_->readLine(line)) {
    uint len = uint(line.size());

    if (len == 0) {
      ++lineNum;
      continue;
    }

    if (line[len - 1] == '\\') {
      if (line1 != "")
        line1 += " " + line.substr(0, len - 1);
      else
        line1 = line.substr(0, len - 1);

      ++lineNum;

      continue;
    }

    if (line1 != "")
      line = line1 + " " + line;

    lines.push_back(Line(line, lineNum));

    line1 = "";
  }

  if (line1 != "")
    lines.push_back(line1);

  if (! lines.empty())
    executeLines(lines);

  return true;
}

bool
Input::
executeStdIn()
{
  historyActive_ = true;

  executeBlockLines(true);

  return true;
}

void
Input::
executeLine(std::string &line)
{
  LineArray lines;

  lines.push_back(Line(line));

  executeLines(lines);
}

void
Input::
executeLines(const LineArray &lines)
{
  auto *block = cwsh_->startBlock(BlockType::FILE, lines);

  block->setFilename(inputFile_->getPath());

  executeBlockLines(false);

  cwsh_->endBlock();
}

void
Input::
executeBlockLines(bool interactive)
{
  while (! eof()) {
    auto cline = getLine();

    if (interactive && cwsh_->getSilentMode()) {
      printf("%s", CEscape::commandToEscape(cline.line, COSFile::getCurrentDir(), true).c_str());
      fflush(stdout);
    }

    processLine(cline);

    if (interactive && cwsh_->getSilentMode()) {
      printf("%s", CEscape::commandToEscape(cline.line, COSFile::getCurrentDir(), false).c_str());
      fflush(stdout);
    }

    if (cwsh_->getExit())
      break;
  }
}

bool
Input::
eof()
{
  if (cwsh_->inBlock())
    return cwsh_->blockEof();

  return false;
}

void
Input::
getBlock(ShellCommand *shellCommand, LineArray &lines)
{
  auto promptType    = cwsh_->getPromptType();
  auto promptCommand = cwsh_->getPromptCommand();

  cwsh_->setPromptType   (PromptType::EXTRA);
  cwsh_->setPromptCommand(shellCommand->getName());

  while (! eof()) {
    auto line = getLine();

    std::vector<std::string> words;

    String::addWords(line.line, words);

    if (words.size() > 0) {
      auto name = words[0];

      if (name == shellCommand->getEndName())
        break;

      lines.push_back(line);

      auto *shellCommand1 = cwsh_->lookupShellCommand(name);

      if (shellCommand1 && shellCommand1->isBlockCommand()) {
        LineArray lines1;

        getBlock(shellCommand1, lines1);

        copy(lines1.begin(), lines1.end(), back_inserter(lines));

        lines.push_back(Line(shellCommand1->getEndName()));
      }
    }
    else
      lines.push_back(line);
  }

  cwsh_->setPromptType   (promptType);
  cwsh_->setPromptCommand(promptCommand);
}

void
Input::
skipBlock(const Line &line)
{
  std::vector<std::string> words;

  String::addWords(line.line, words);

  if (words.size() == 0)
    return;

  auto *shellCommand = cwsh_->lookupShellCommand(words[0]);

  if (! shellCommand || ! shellCommand->isBlockCommand())
    return;

  LineArray lines;

  getBlock(shellCommand, lines);
}

Line
Input::
getLine()
{
  if (cwsh_->inBlock())
    return cwsh_->blockReadLine();

  std::string line;

  if (inputFile_->isStdIn()) {
    line = cwsh_->readLine();

    if (line.size() > 0 && line[line.size() - 1] == '\\') {
      auto promptType    = cwsh_->getPromptType();
      auto promptCommand = cwsh_->getPromptCommand();

      cwsh_->setPromptType   (PromptType::EXTRA);
      cwsh_->setPromptCommand("");

      while (line.size() > 0 && line[line.size() - 1] == '\\') {
        auto line1 = cwsh_->readLine();

        line = line.substr(0, line.size() - 1) + "\n" + line1;
      }

      cwsh_->setPromptType   (promptType);
      cwsh_->setPromptCommand(promptCommand);
    }

    cwsh_->displayExitedProcesses();
  }
  else {
    line = String::readLineFromFile(inputFile_);

    while (line.size() > 0 && line[line.size() - 1] == '\\') {
      auto line1 = String::readLineFromFile(inputFile_);

      line = line.substr(0, line.size() - 1) + "\n" + line1;
    }
  }

  return line;
}

std::string
Input::
getFilename() const
{
  if (! inputFile_)
    return "";

  return inputFile_->getPath();
}

void
Input::
processLine(const Line &line)
{
  cwsh_->setFilename(getFilename());
  cwsh_->setLineNum (line.num);

  try {
    std::string line1;

    //------

    // Check for comment as first character

    uint i = 0;

    CStrUtil::skipSpace(line.line, &i);

    uint len = uint(line.line.size());

    if (i < len && line.line[i] != '#')
      line1 = line.line;

    //------

    // History Substitution

    bool output = false;

    HistoryParser parser(cwsh_);

    auto line2 = parser.parseLine(line1);

    if (line2 != line1) {
      output = true;

      line1 = line2;
    }

    //------

    // Remove Extra Spaces

    std::vector<std::string> words1;

    String::addWords(line1, words1);

    line1 = CStrUtil::toString(words1, " ");

    //------

    // Output Line if Verbose

    auto *variable = cwsh_->lookupVariable("verbose");

    if (variable)
      output = true;

    if (output)
      std::cout << line1 << "\n";

    //------

    // Set Current History Command

    cwsh_->setHistoryCurrent(line1);

    //------

    // Ignore if Blank Line or Comment

    if (line1 == "")
      return;

    //------

    // Add Command To History

    if (historyActive_)
      cwsh_->addHistoryCommand(line1);

    //------

    // Convert Line to Command Lines

    CmdGroupArray groups;

    CommandUtil::parseCommandGroups(cwsh_, line1, groups);

    //------

    uint numGroups = uint(groups.size());

    for (uint ig = 0; ig < numGroups; ig++) {
      // Parse Command

      auto cmds = CommandUtil::parseCommandGroup(cwsh_, groups[ig].get());

      // Execute Commands

      executeCommands(cmds);
    }
  }
  catch (HistoryIgnore i) {
    ;
  }
  catch (struct Err *cthrow) {
    auto qualifier = cthrow->qualifier;

    if (qualifier == "" && currentCommand_)
      qualifier = currentCommand_->getCommand()->getName();

    if (cwsh_->getDebug())
      std::cerr << "[" << cthrow->file << ":" << cthrow->line << "] ";

    if (qualifier != "")
      std::cerr << qualifier << ": " << cthrow->message << "\n";
    else
      std::cerr << cthrow->message << "\n";
  }
  catch (...) {
    std::cerr << "Unhandled Exception thrown\n";
  }
}

void
Input::
executeCommands(const CmdArray &cmds)
{
  std::vector<CommandData *> pcommands;
  std::vector<std::string>   deleteFiles;
  CommandData*               firstCommand = nullptr;

  int numCmds = int(cmds.size());

  for (int i = 0; i < numCmds; i++) {
    if (cwsh_->getDebug()) {
      std::cerr << "Execute Command: ";

      cmds[i]->display();
    }

    //-----

    // Get Command

    std::vector<std::string> words;

    int numWords = cmds[i]->getNumWords();

    for (int j = 0; j < numWords; j++)
      words.push_back(cmds[i]->getWord(j).getWord());

    auto *command = new CommandData(cwsh_, words);

    auto *ccommand = command->getCommand();

    if (! ccommand) {
      delete command;
      continue;
    }

    currentCommand_ = command;

    //-----

    // Echo Command

    auto *variable = cwsh_->lookupVariable("echo");

    if (variable)
      std::cout << ccommand->getCommandString() << "\n";

    //-----

    // Set Redirection

    if      (cmds[i]->hasStdInToken()) {
      const auto &filename = readStdInToken(cmds[i]->getStdInToken());

      ccommand->addFileSrc(filename);

      deleteFiles.push_back(filename);
    }
    else if (cmds[i]->hasStdInFile())
      ccommand->addFileSrc(cmds[i]->getStdInFile());

    if (cmds[i]->hasStdOutFile()) {
      ccommand->addFileDest(cmds[i]->getStdOutFile(), 1);

      auto *variable1 = cwsh_->lookupVariable("noclobber");

      if (cmds[i]->getStdOutClobber() || ! variable1)
        ccommand->setFileDestOverwrite(true, 1);
      else
        ccommand->setFileDestOverwrite(false, 1);

      if (cmds[i]->getStdOutAppend())
        ccommand->setFileDestAppend(true, 1);
    }

    if (cmds[i]->hasStdErrFile()) {
      if (! cmds[i]->hasStdOutFile()) {
        ccommand->addFileDest(cmds[i]->getStdErrFile(), 1);

        auto *variable1 = cwsh_->lookupVariable("noclobber");

        if (cmds[i]->getStdErrClobber() || ! variable1)
          ccommand->setFileDestOverwrite(true, 1);
        else
          ccommand->setFileDestOverwrite(false, 1);

        if (cmds[i]->getStdErrAppend())
          ccommand->setFileDestAppend(true, 1);
      }

      ccommand->addFileDest(cmds[i]->getStdErrFile(), 2);

      auto *variable2 = cwsh_->lookupVariable("noclobber");

      if (cmds[i]->getStdErrClobber() || ! variable2)
        ccommand->setFileDestOverwrite(true, 2);
      else
        ccommand->setFileDestOverwrite(false, 2);

      if (cmds[i]->getStdErrAppend())
        ccommand->setFileDestAppend(true, 2);
    }

    // Run Command

    auto separator = cmds[i]->getSeparator().getType();

    if (separator != CmdSeparatorType::PIPE && separator != CmdSeparatorType::PIPE_ERR) {
      auto numPCommands = pcommands.size();

      if (numPCommands > 0) {
        ccommand->addPipeSrc();

        for (uint k = 0; k < numPCommands - 1; k++) {
          auto *pccommand = pcommands[k]->getCommand();

          if (! firstCommand) {
            firstCommand = pcommands[k];

            pccommand->setProcessGroupLeader();
          }
          else
            pccommand->setProcessGroup(firstCommand->getCommand());

          pccommand->start();
        }

        auto *pccommand = pcommands[numPCommands - 1]->getCommand();

        if (! firstCommand) {
          firstCommand = pcommands[numPCommands - 1];

          pccommand->setProcessGroupLeader();
        }
        else
          pccommand->setProcessGroup(firstCommand->getCommand());

        pccommand->start();

        ccommand->setProcessGroup(firstCommand->getCommand());

        ccommand->start();

        //------

        auto *process = cwsh_->addProcess(firstCommand);

        for (uint k = 0; k < numPCommands; k++)
          if (pcommands[k] != firstCommand)
            process->addSubCommand(pcommands[k]);

        if (firstCommand != command)
          process->addSubCommand(command);

        //------

        if (separator != CmdSeparatorType::BACKGROUND) {
          for (uint k = 0; k < numPCommands; k++) {
            auto *pccommand1 = pcommands[k]->getCommand();

            pccommand1->wait();
          }

          ccommand->wait();

          int status = ccommand->getReturnCode();

          cwsh_->defineVariable("status", status);

          for (auto &pcommand : pcommands)
            delete pcommand;

          cwsh_->removeProcess(process);

          currentCommand_ = nullptr;

          if (separator == CmdSeparatorType::AND && status != 0)
            break;

          if (separator == CmdSeparatorType::OR && status == 0)
            break;
        }
        else {
          std::cout << "[" << process->getNum() << "]";

          for (uint k = 0; k < numPCommands; k++) {
            auto *pccommand1 = pcommands[k]->getCommand();

            std::cout << " " << pccommand1->getPid();
          }

          std::cout << " " << ccommand->getPid() << "\n";
        }

        pcommands.clear();
      }
      else {
        if (ccommand->getDoFork()) {
          if (! firstCommand) {
            firstCommand = command;

            ccommand->setProcessGroupLeader();
          }
          else
            ccommand->setProcessGroup(firstCommand->getCommand());
        }

        ccommand->start();

        auto *process = cwsh_->addProcess(firstCommand);

        if (separator != CmdSeparatorType::BACKGROUND) {
          ccommand->wait();

          if (ccommand->getState() == CCommand::State::EXITED) {
            int status = ccommand->getReturnCode();

            cwsh_->defineVariable("status", status);

            cwsh_->removeProcess(process);

            currentCommand_ = nullptr;

            if (separator == CmdSeparatorType::AND && status != 0)
              break;

            if (separator == CmdSeparatorType::OR && status == 0)
              break;
          }
          else {
            int pid = ccommand->getPid();

            std::cout << "[" << process->getNum() << "] " << pid << "\n";
          }
        }
        else {
          int pid = ccommand->getPid();

          std::cout << "[" << process->getNum() << "] " << pid << "\n";
        }
      }
    }
    else {
      if (pcommands.size() > 0)
        ccommand->addPipeSrc();

      ccommand->addPipeDest(1);

      if (separator == CmdSeparatorType::PIPE_ERR)
        ccommand->addPipeDest(2);

      pcommands.push_back(command);
    }
  }

  auto numDeleteFiles = deleteFiles.size();

  for (uint i = 0; i < numDeleteFiles; i++)
    unlink(deleteFiles[i].c_str());
}

std::string
Input::
readStdInToken(const std::string &token)
{
  CTempFile tempFile;

  cwsh_->setPromptType(PromptType::EXTRA);

  while (1) {
    auto line = cwsh_->readLine();

    if (line == token)
      break;

    if (token[0] != '"') {
      auto line1 = processStdInLine(line);

      tempFile.getFile()->write(line1 + "\n");
    }
    else
      tempFile.getFile()->write(line + "\n");
  }

  tempFile.getFile()->close();

  cwsh_->setPromptType(PromptType::NORMAL);

  return tempFile.getFile()->getPath();
}

std::string
Input::
processStdInLine(const Line &line)
{
  WordArray words;

  Word::toWords(line.line, words);

  if (cwsh_->getDebug()) {
    std::cerr << "Std In Line to Words\n";

    Word::printWords(words);
  }

  int numWords = int(words.size());

  //------

  // Replace Variables

  WordArray variableWords;

  for (int i = 0; i < numWords; i++) {
    WordArray words2;

    VariableParser vparser(cwsh_, words[i]);

    if (vparser.expandVariables(words2))
      copy(words2.begin(), words2.end(), back_inserter(variableWords));
    else
      variableWords.push_back(words[i]);
  }

  int numVariableWords = int(variableWords.size());

  //------

  // Replace backquotes

  WordArray cmdWords;

  for (int i = 0; i < numVariableWords; i++) {
    const auto &word = variableWords[i];

    WordArray inPlaceWords;

    InPlaceCommand icmd(cwsh_, word);

    if (icmd.expand(inPlaceWords)) {
      auto numInPlaceWords = inPlaceWords.size();

      for (uint k = 0; k < numInPlaceWords; ++k)
        cmdWords.push_back(inPlaceWords[k]);
    }
    else
      cmdWords.push_back(word);
  }

  //------

  return Word::toString(cmdWords);
}

std::string
Input::
processExprLine(const Line &line)
{
  WordArray words;

  Word::toWords(line.line, words);

  if (cwsh_->getDebug()) {
    std::cerr << "Expr Line to Words\n";

    Word::printWords(words);
  }

  int numWords = int(words.size());

  //------

  // Replace Variables

  WordArray variableWords;

  for (int i = 0; i < numWords; i++) {
    WordArray twords;

    VariableParser vparser(cwsh_, words[i]);

    if (vparser.expandVariables(twords))
      copy(twords.begin(), twords.end(), back_inserter(variableWords));
    else
      variableWords.push_back(words[i]);
  }

  int numVariableWords = int(variableWords.size());

  //------

  // Replace backquotes

  WordArray cmdWords;

  for (int i = 0; i < numVariableWords; i++) {
    const auto &word = variableWords[i];

    WordArray twords;

    InPlaceCommand icmd(cwsh_, variableWords[i]);

    if (icmd.expand(twords))
      copy(twords.begin(), twords.end(), back_inserter(cmdWords));
    else
      cmdWords.push_back(word);
  }

  int numCmdWords = int(cmdWords.size());

  //------

  // Expand Tildes

  for (int i = 0; i < numCmdWords; i++) {
    std::string str;

    if (CFile::expandTilde(cmdWords[i].getWord(), str))
      cmdWords[i] = Word(str);
  }

  //------

  // Expand Braces

  WordArray braceWords;

  for (int i = 0; i < numCmdWords; i++) {
    const auto &word = cmdWords[i];

    WordArray twords;

    if (Braces::expand(word, twords))
      copy(twords.begin(), twords.end(), back_inserter(braceWords));
    else
      braceWords.push_back(word);
  }

  int numBraceWords = int(braceWords.size());

  //------

  // Expand Wildcards

  WordArray wildcardWords;

  for (int i = 0; i < numBraceWords; i++) {
    const auto &word = braceWords[i];

    WordArray twords;

    Pattern pattern(cwsh_);

    if (pattern.expandWordToFiles(word, twords))
      copy(twords.begin(), twords.end(), back_inserter(wildcardWords));
    else
      wildcardWords.push_back(word);
  }

  //------

  return Word::toString(wildcardWords);
}

std::string
Input::
getPrompt()
{
  Variable *promptVar;

  if (cwsh_->getPromptType() == PromptType::NORMAL)
    promptVar = cwsh_->lookupVariable("prompt");
  else
    promptVar = cwsh_->lookupVariable("prompt1");

  auto *colorVar = cwsh_->lookupVariable("prompt_color");

  std::string promptString;

  if (cwsh_->getPromptType() != PromptType::NORMAL)
    promptString += cwsh_->getPromptCommand();

  if (promptVar)
    promptString += promptVar->getValue(0);
  else {
    if (cwsh_->getPromptType() == PromptType::NORMAL)
      promptString += "> ";
    else
      promptString += "? ";
  }

  std::string promptString1;

  CStrParse parse(promptString);

  while (! parse.eof()) {
    if (parse.isChar('%')) {
      bool processed = false;

      parse.skipChar();

      auto c = parse.readChar();

      if (parse.isChar('%')) {
        parse.skipChar();

        processed = true;

        if      (c == 'B')
          promptString1 += "[1m";
        else if (c == 'b')
          promptString1 += "[0m";
        else
          processed = false;

        if (! processed) {
          promptString1 += "%" + c;
          promptString1 += "%";
        }
      }
      else
        promptString1 += "%" + c;
    }
    else
      promptString1 += parse.readChar();
  }

  if (colorVar) {
    CRGBA c;

    if (CRGBName::toRGBA(colorVar->getValue(0), c)) {
      auto colorStr = CEscapeColorsInst->colorFgStr(c);

      promptString1 = colorStr + promptString1 + "[0m";
    }
  }

  return promptString1;
}

}
