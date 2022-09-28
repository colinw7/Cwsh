#include <CwshI.h>

bool
CwshCmdSplit::
wordsToCommandLines(const CwshWordArray &words, CwshCmdLineArray &cmds)
{
  int i = 0;

  int num_words = int(words.size());

  while (i < num_words) {
    CwshCmdLine *cmd = new CwshCmdLine();

    wordsToCommandLine(words, &i, cmd);

    cmds.push_back(cmd);
  }

  return true;
}

void
CwshCmdSplit::
wordsToCommandLine(const CwshWordArray &words, int *i, CwshCmdLine *cmd)
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
CwshCmdSplit::
wordsToCommands(const CwshWordArray &words, CwshCmdArray &cmds)
{
  int i = 0;

  int num_words = int(words.size());

  while (i < num_words) {
    CwshCmd *cmd = new CwshCmd();

    wordsToCommand(words, &i, cmd);

    cmds.push_back(cmd);
  }

  //------

  if (cmds.size() > 0) {
    CwshCmd *last_cmd = cmds[cmds.size() - 1];

    CwshCmdSeparatorType separator_type = last_cmd->getSeparator().getType();

    if (separator_type == CwshCmdSeparatorType::PIPE     ||
        separator_type == CwshCmdSeparatorType::PIPE_ERR ||
        separator_type == CwshCmdSeparatorType::AND      ||
        separator_type == CwshCmdSeparatorType::OR)
      CWSH_THROW("Invalid null command.");
  }

  return true;
}

void
CwshCmdSplit::
wordsToCommand(const CwshWordArray &words, int *i, CwshCmd *cmd)
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
      CwshCmdSeparator separator = parseCommandSeparator(word);

      if (separator.getType() != CwshCmdSeparatorType::NONE) {
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

CwshCmdSeparator
CwshCmdSplit::
parseCommandSeparator(const std::string &word)
{
  if      (word == "&")
    return CwshCmdSeparator(CwshCmdSeparatorType::BACKGROUND);
  else if (word == "|")
    return CwshCmdSeparator(CwshCmdSeparatorType::PIPE);
  else if (word == "|&")
    return CwshCmdSeparator(CwshCmdSeparatorType::PIPE_ERR);
  else if (word == "&&")
    return CwshCmdSeparator(CwshCmdSeparatorType::AND);
  else if (word == "||")
    return CwshCmdSeparator(CwshCmdSeparatorType::OR);
  else if (word == ";")
    return CwshCmdSeparator(CwshCmdSeparatorType::NORMAL);
  else
    return CwshCmdSeparator(CwshCmdSeparatorType::NONE);
}

CwshCmdLine::
CwshCmdLine()
{
}

CwshCmdLine::
~CwshCmdLine()
{
}

void
CwshCmdLine::
addWord(const CwshWord &word)
{
  words_.push_back(word);
}

CwshCmdGroup::
CwshCmdGroup(const CwshCmdArray &commands) :
 commands_(commands)
{
}

CwshCmdGroup::
~CwshCmdGroup()
{
  for (auto &command : commands_)
    delete command;
}

void
CwshCmd::
displayCmdArray(const CwshCmdArray &cmds)
{
  for (auto &cmd : cmds)
    CwshCmd::displayCmd(cmd);
}

void
CwshCmd::
displayCmd(const CwshCmd *cmd)
{
  cmd->display();
}

CwshCmd::
CwshCmd() :
 separator_(CwshCmdSeparatorType::NONE)
{
}

CwshCmd::
~CwshCmd()
{
}

void
CwshCmd::
addWord(const CwshWord &word)
{
  words_.push_back(word);
}

void
CwshCmd::
setWord(int i, const CwshWord &word)
{
  words_[i] = word;
}

void
CwshCmd::
setSeparator(const CwshCmdSeparator &separator)
{
  separator_ = separator;
}

void
CwshCmd::
setWords(const CwshWordArray &words)
{
  words_.clear();

  copy(words.begin(), words.end(), back_inserter(words_));
}

void
CwshCmd::
display() const
{
  std::string command_str = CwshWord::toString(words_);

  std::cerr << command_str << " " << separator_.getName() << std::endl;
}

std::string
CwshCmdSeparator::
getName() const
{
  if      (type_ == CwshCmdSeparatorType::BACKGROUND)
    return "&";
  else if (type_ == CwshCmdSeparatorType::PIPE)
    return "|";
  else if (type_ == CwshCmdSeparatorType::PIPE_ERR)
    return "|&";
  else if (type_ == CwshCmdSeparatorType::AND)
    return "&&";
  else if (type_ == CwshCmdSeparatorType::OR)
    return "||";
  else
    return "";
}
