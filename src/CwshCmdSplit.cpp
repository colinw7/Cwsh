#include <CwshI.h>

namespace Cwsh {

bool
CmdSplit::
wordsToCommandLines(const WordArray &words, CmdLineArray &cmds)
{
  int i = 0;

  int num_words = int(words.size());

  while (i < num_words) {
    auto *cmd = new CmdLine();

    wordsToCommandLine(words, &i, cmd);

    cmds.push_back(cmd);
  }

  return true;
}

void
CmdSplit::
wordsToCommandLine(const WordArray &words, int *i, CmdLine *cmd)
{
  int brackets = 0;

  int num_words = int(words.size());

  while (*i < num_words) {
    const std::string &word = words[*i].getWord();

    if      (word == "(")
      ++brackets;
    else if (word == ")") {
      --brackets;

      if (brackets < 0)
        CWSH_THROW("Too many )'s.");
    }
    else if (word == ";") {
      ++(*i);

      break;
    }

    cmd->addWord(words[*i]);

    ++(*i);
  }

  if (brackets > 0)
    CWSH_THROW("Too many ('s.");
}

bool
CmdSplit::
wordsToCommands(const WordArray &words, CmdArray &cmds)
{
  int i = 0;

  int num_words = int(words.size());

  while (i < num_words) {
    auto *cmd = new Cmd();

    wordsToCommand(words, &i, cmd);

    cmds.push_back(cmd);
  }

  //------

  if (cmds.size() > 0) {
    Cmd *last_cmd = cmds[cmds.size() - 1];

    auto separator_type = last_cmd->getSeparator().getType();

    if (separator_type == CmdSeparatorType::PIPE     ||
        separator_type == CmdSeparatorType::PIPE_ERR ||
        separator_type == CmdSeparatorType::AND      ||
        separator_type == CmdSeparatorType::OR)
      CWSH_THROW("Invalid null command.");
  }

  return true;
}

void
CmdSplit::
wordsToCommand(const WordArray &words, int *i, Cmd *cmd)
{
  int brackets = 0;

  int num_words = int(words.size());

  while (*i < num_words) {
    const std::string &word = words[*i].getWord();

    if      (word == "(")
      ++brackets;
    else if (word == ")") {
      --brackets;

      if (brackets < 0)
        CWSH_THROW("Too many )'s.");
    }
    else if (brackets == 0) {
      auto separator = parseCommandSeparator(word);

      if (separator.getType() != CmdSeparatorType::NONE) {
        cmd->setSeparator(separator);

        ++(*i);

        break;
      }
    }

    cmd->addWord(words[*i]);

    ++(*i);
  }

  if (cmd->getNumWords() == 0)
    CWSH_THROW("Invalid null command.");

  if (brackets > 0)
    CWSH_THROW("Too many ('s.");
}

CmdSeparator
CmdSplit::
parseCommandSeparator(const std::string &word)
{
  if      (word == "&")
    return CmdSeparator(CmdSeparatorType::BACKGROUND);
  else if (word == "|")
    return CmdSeparator(CmdSeparatorType::PIPE);
  else if (word == "|&")
    return CmdSeparator(CmdSeparatorType::PIPE_ERR);
  else if (word == "&&")
    return CmdSeparator(CmdSeparatorType::AND);
  else if (word == "||")
    return CmdSeparator(CmdSeparatorType::OR);
  else if (word == ";")
    return CmdSeparator(CmdSeparatorType::NORMAL);
  else
    return CmdSeparator(CmdSeparatorType::NONE);
}

CmdLine::
CmdLine()
{
}

CmdLine::
~CmdLine()
{
}

void
CmdLine::
addWord(const Word &word)
{
  words_.push_back(word);
}

CmdGroup::
CmdGroup(const CmdArray &commands) :
 commands_(commands)
{
}

CmdGroup::
~CmdGroup()
{
  for (auto &command : commands_)
    delete command;
}

void
Cmd::
displayCmdArray(const CmdArray &cmds)
{
  for (auto &cmd : cmds)
    Cmd::displayCmd(cmd);
}

void
Cmd::
displayCmd(const Cmd *cmd)
{
  cmd->display();
}

Cmd::
Cmd() :
 separator_(CmdSeparatorType::NONE)
{
}

Cmd::
~Cmd()
{
}

void
Cmd::
addWord(const Word &word)
{
  words_.push_back(word);
}

void
Cmd::
setWord(int i, const Word &word)
{
  words_[i] = word;
}

void
Cmd::
setSeparator(const CmdSeparator &separator)
{
  separator_ = separator;
}

void
Cmd::
setWords(const WordArray &words)
{
  words_.clear();

  copy(words.begin(), words.end(), back_inserter(words_));
}

void
Cmd::
display() const
{
  auto command_str = Word::toString(words_);

  std::cerr << command_str << " " << separator_.getName() << "\n";
}

std::string
CmdSeparator::
getName() const
{
  if      (type_ == CmdSeparatorType::BACKGROUND)
    return "&";
  else if (type_ == CmdSeparatorType::PIPE)
    return "|";
  else if (type_ == CmdSeparatorType::PIPE_ERR)
    return "|&";
  else if (type_ == CmdSeparatorType::AND)
    return "&&";
  else if (type_ == CmdSeparatorType::OR)
    return "||";
  else
    return "";
}

}
