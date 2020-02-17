#include <CwshI.h>

CwshInPlaceCommand::
CwshInPlaceCommand(Cwsh *cwsh, const CwshWord &word) :
 cwsh_(cwsh), word_(word)
{
}

bool
CwshInPlaceCommand::
expand(CwshWordArray &words)
{
  bool flag = false;

  const CwshSubWordArray &sub_words = word_.getSubWords();

  std::string str1;

  int num_sub_words = sub_words.size();

  for (int i = 0; i < num_sub_words; i++) {
    CwshSubWordType type = sub_words[i].getType();

    std::string sub_str;

    if      (type == CwshSubWordType::BACK_QUOTED) {
      sub_str = expandCommand(sub_words[i].getWord());

      flag = true;
    }
    else if (type == CwshSubWordType::DOUBLE_QUOTED) {
      std::string sub_str1;

      if (expandQuotedWord(CwshWord(sub_words[i].getWord()), sub_str1)) {
        sub_str = '"' + sub_str1 + '"';

        flag = true;
      }
      else
        sub_str = sub_words[i].getString();
    }
    else
      sub_str = sub_words[i].getString();

    str1 += sub_str;
  }

  if (flag) {
    CwshWord::toWords(str1, words);

    if (cwsh_->getDebug()) {
      std::cerr << "Expand In Place String to Words" << std::endl;

      CwshWord::printWords(words);
    }
  }

  return flag;
}

bool
CwshInPlaceCommand::
expandQuotedWord(const CwshWord &word, std::string &word1)
{
  bool flag = false;

  const CwshSubWordArray &sub_words = word.getSubWords();

  int num_sub_words = sub_words.size();

  for (int i = 0; i < num_sub_words; i++) {
    CwshSubWordType type = sub_words[i].getType();

    std::string sub_word;

    if (type == CwshSubWordType::BACK_QUOTED) {
      sub_word = expandCommand(sub_words[i].getWord());

      flag = true;
    }
    else
      sub_word = sub_words[i].getString();

    word1 += sub_word;
  }

  return flag;
}

std::string
CwshInPlaceCommand::
expandCommand(const std::string &str)
{
  // Convert Line to Command Groups

  CwshCmdGroupArray groups;

  CwshCommandUtil::parseCommandGroups(cwsh_, str, groups);

  //------

  std::string output;

  int num_groups = groups.size();

  for (int i = 0; i < num_groups; i++) {
    CwshCmdArray cmds = CwshCommandUtil::parseCommandGroup(cwsh_, groups[i]);

    //------

    // Execute Commands

    output = executeCommands(cmds);
  }

  //------

  for (auto &group : groups)
    delete group;

  //------

  if (cwsh_->getDebug()) {
    std::cerr << "Expand Command '" << str << "' to '" << output << "'" << std::endl;
  }

  return output;
}

std::string
CwshInPlaceCommand::
executeCommands(const CwshCmdArray &cmds)
{
  std::string output;

  //------

  // Execute Commands

  std::vector<CwshCommandData *> pcommands;

  int num_cmds = cmds.size();

  for (int i = 0; i < num_cmds; i++) {
    if (cwsh_->getDebug()) {
      std::cerr << "Execute Command: ";

      cmds[i]->display();
    }

    CwshCmdSeparatorType separator = cmds[i]->getSeparator().getType();

    // Get Command

    std::vector<std::string> words;

    int num_words = cmds[i]->getNumWords();

    for (int j = 0; j < num_words; j++)
      words.push_back(cmds[i]->getWord(j).getWord());

    CAutoPtr<CwshCommandData> command;

    command = new CwshCommandData(cwsh_, words);

    CwshCommand *command1 = command->getCommand();

    if (! command1)
      continue;

    // Set Redirection

    if (cmds[i]->hasStdInFile())
      command1->addFileSrc(cmds[i]->getStdInFile());

    if (cmds[i]->hasStdOutFile()) {
      command1->addFileDest(cmds[i]->getStdOutFile(), 1);

      CwshVariable *variable = cwsh_->lookupVariable("noclobber");

      if (cmds[i]->getStdOutClobber() || ! variable)
        command1->setFileDestOverwrite(true, 1);
      else
        command1->setFileDestOverwrite(false, 1);

      if (cmds[i]->getStdOutAppend())
        command1->setFileDestAppend(true, 1);
    }

    if (cmds[i]->hasStdErrFile()) {
      if (! cmds[i]->hasStdOutFile()) {
        command1->addFileDest(cmds[i]->getStdErrFile(), 1);

        CwshVariable *variable = cwsh_->lookupVariable("noclobber");

        if (cmds[i]->getStdErrClobber() || ! variable)
          command1->setFileDestOverwrite(true, 1);
        else
          command1->setFileDestOverwrite(false, 1);

        if (cmds[i]->getStdErrAppend())
          command1->setFileDestAppend(true, 1);
      }

      command1->addFileDest(cmds[i]->getStdErrFile(), 2);

      CwshVariable *variable = cwsh_->lookupVariable("noclobber");

      if (cmds[i]->getStdErrClobber() || ! variable)
        command1->setFileDestOverwrite(true, 2);
      else
        command1->setFileDestOverwrite(false, 2);

      if (cmds[i]->getStdErrAppend())
        command1->setFileDestAppend(true, 2);
    }

    // Run Command

    if (separator != CwshCmdSeparatorType::PIPE &&
        separator != CwshCmdSeparatorType::PIPE_ERR) {
      int num_pcommands = pcommands.size();

      if (num_pcommands > 0) {
        command1->addPipeSrc();

        for (int k = 0; k < num_pcommands - 1; k++)
          pcommands[k]->getCommand()->start();

        pcommands[num_pcommands - 1]->getCommand()->start();

        int numPCmds = cmds.size();

        if (i == numPCmds - 1) {
          command1->addStringDest(output);

          separator = CwshCmdSeparatorType::NORMAL;
        }

        command1->start();

        for (int k = 0; k < num_pcommands; k++)
          pcommands[k]->getCommand()->wait();

        if (separator != CwshCmdSeparatorType::BACKGROUND) {
          command1->wait();

          int status = command1->getReturnCode();

          cwsh_->defineVariable("status", status);

          for (auto &pcommand : pcommands)
            delete pcommand;

          if (separator == CwshCmdSeparatorType::AND && status != 0)
            break;

          if (separator == CwshCmdSeparatorType::OR && status == 0)
            break;
        }
        else {
          CwshProcess *process = cwsh_->addProcess(command.release());

          int pid = command1->getPid();

          std::cout << "[" << process->getNum() << "] " << pid << std::endl;
        }

        pcommands.clear();
      }
      else {
        int numPCmds = cmds.size();

        if (i == numPCmds - 1) {
          command1->addStringDest(output);

          separator = CwshCmdSeparatorType::NORMAL;
        }

        command1->start();

        if (separator != CwshCmdSeparatorType::BACKGROUND) {
          command1->wait();

          int status = command1->getReturnCode();

          cwsh_->defineVariable("status", status);

          if (separator == CwshCmdSeparatorType::AND && status != 0)
            break;

          if (separator == CwshCmdSeparatorType::OR && status == 0)
            break;
        }
        else {
          CwshProcess *process = cwsh_->addProcess(command.release());

          int pid = command1->getPid();

          std::cout << "[" << process->getNum() << "] " << pid << std::endl;
        }
      }
    }
    else {
      if (pcommands.size() > 0)
        command1->addPipeSrc();

      if (separator == CwshCmdSeparatorType::PIPE)
        command1->addPipeDest(1);
      else {
        command1->addPipeDest(1);
        command1->addPipeDest(2);
      }

      pcommands.push_back(command.release());
    }
  }

  return CStrUtil::compressSpaces(output);
}
