#include <CwshI.h>

namespace Cwsh {

InPlaceCommand::
InPlaceCommand(App *cwsh, const Word &word) :
 cwsh_(cwsh), word_(word)
{
}

bool
InPlaceCommand::
expand(WordArray &words)
{
  bool flag = false;

  const auto &sub_words = word_.getSubWords();

  std::string str1;

  int num_sub_words = int(sub_words.size());

  for (int i = 0; i < num_sub_words; i++) {
    auto type = sub_words[i].getType();

    std::string sub_str;

    if      (type == SubWordType::BACK_QUOTED) {
      sub_str = expandCommand(sub_words[i].getWord());

      flag = true;
    }
    else if (type == SubWordType::DOUBLE_QUOTED) {
      std::string sub_str1;

      if (expandQuotedWord(Word(sub_words[i].getWord()), sub_str1)) {
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
    Word::toWords(str1, words);

    if (cwsh_->getDebug()) {
      std::cerr << "Expand In Place String to Words\n";

      Word::printWords(words);
    }
  }

  return flag;
}

bool
InPlaceCommand::
expandQuotedWord(const Word &word, std::string &word1)
{
  bool flag = false;

  const auto &sub_words = word.getSubWords();

  int num_sub_words = int(sub_words.size());

  for (int i = 0; i < num_sub_words; i++) {
    auto type = sub_words[i].getType();

    std::string sub_word;

    if (type == SubWordType::BACK_QUOTED) {
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
InPlaceCommand::
expandCommand(const std::string &str)
{
  // Convert Line to Command Groups

  CmdGroupArray groups;

  CommandUtil::parseCommandGroups(cwsh_, str, groups);

  //------

  std::string output;

  int num_groups = int(groups.size());

  for (int i = 0; i < num_groups; i++) {
    auto cmds = CommandUtil::parseCommandGroup(cwsh_, groups[i].get());

    //------

    // Execute Commands

    output = executeCommands(cmds);
  }

  //------

  if (cwsh_->getDebug()) {
    std::cerr << "Expand Command '" << str << "' to '" << output << "'\n";
  }

  return output;
}

std::string
InPlaceCommand::
executeCommands(const CmdArray &cmds)
{
  std::string output;

  //------

  // Execute Commands

  std::vector<CommandData *> pcommands;

  int num_cmds = int(cmds.size());

  for (int i = 0; i < num_cmds; i++) {
    if (cwsh_->getDebug()) {
      std::cerr << "Execute Command: ";

      cmds[i]->display();
    }

    auto separator = cmds[i]->getSeparator().getType();

    // Get Command

    std::vector<std::string> words;

    int num_words = cmds[i]->getNumWords();

    for (int j = 0; j < num_words; j++)
      words.push_back(cmds[i]->getWord(j).getWord());

    using CommandDataP = std::unique_ptr<CommandData>;

    CommandDataP command;

    command = std::make_unique<CommandData>(cwsh_, words);

    auto *command1 = command->getCommand();

    if (! command1)
      continue;

    // Set Redirection

    if (cmds[i]->hasStdInFile())
      command1->addFileSrc(cmds[i]->getStdInFile());

    if (cmds[i]->hasStdOutFile()) {
      command1->addFileDest(cmds[i]->getStdOutFile(), 1);

      auto *variable = cwsh_->lookupVariable("noclobber");

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

        auto *variable = cwsh_->lookupVariable("noclobber");

        if (cmds[i]->getStdErrClobber() || ! variable)
          command1->setFileDestOverwrite(true, 1);
        else
          command1->setFileDestOverwrite(false, 1);

        if (cmds[i]->getStdErrAppend())
          command1->setFileDestAppend(true, 1);
      }

      command1->addFileDest(cmds[i]->getStdErrFile(), 2);

      auto *variable = cwsh_->lookupVariable("noclobber");

      if (cmds[i]->getStdErrClobber() || ! variable)
        command1->setFileDestOverwrite(true, 2);
      else
        command1->setFileDestOverwrite(false, 2);

      if (cmds[i]->getStdErrAppend())
        command1->setFileDestAppend(true, 2);
    }

    // Run Command

    if (separator != CmdSeparatorType::PIPE && separator != CmdSeparatorType::PIPE_ERR) {
      int num_pcommands = int(pcommands.size());

      if (num_pcommands > 0) {
        command1->addPipeSrc();

        for (int k = 0; k < num_pcommands - 1; k++)
          pcommands[k]->getCommand()->start();

        pcommands[num_pcommands - 1]->getCommand()->start();

        int numPCmds = int(cmds.size());

        if (i == numPCmds - 1) {
          command1->addStringDest(output);

          separator = CmdSeparatorType::NORMAL;
        }

        command1->start();

        for (int k = 0; k < num_pcommands; k++)
          pcommands[k]->getCommand()->wait();

        if (separator != CmdSeparatorType::BACKGROUND) {
          command1->wait();

          int status = command1->getReturnCode();

          cwsh_->defineVariable("status", status);

          for (auto &pcommand : pcommands)
            delete pcommand;

          if (separator == CmdSeparatorType::AND && status != 0)
            break;

          if (separator == CmdSeparatorType::OR && status == 0)
            break;
        }
        else {
          auto *process = cwsh_->addProcess(command.release());

          int pid = command1->getPid();

          std::cout << "[" << process->getNum() << "] " << pid << "\n";
        }

        pcommands.clear();
      }
      else {
        int numPCmds = int(cmds.size());

        if (i == numPCmds - 1) {
          command1->addStringDest(output);

          separator = CmdSeparatorType::NORMAL;
        }

        command1->start();

        if (separator != CmdSeparatorType::BACKGROUND) {
          command1->wait();

          int status = command1->getReturnCode();

          cwsh_->defineVariable("status", status);

          if (separator == CmdSeparatorType::AND && status != 0)
            break;

          if (separator == CmdSeparatorType::OR && status == 0)
            break;
        }
        else {
          auto *process = cwsh_->addProcess(command.release());

          int pid = command1->getPid();

          std::cout << "[" << process->getNum() << "] " << pid << "\n";
        }
      }
    }
    else {
      if (pcommands.size() > 0)
        command1->addPipeSrc();

      if (separator == CmdSeparatorType::PIPE)
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

}
