#include "CwshI.h"

bool
CwshCmdSplit::
wordsToCommandLines(const CwshWordArray &words, CwshCmdLineArray &cmds)
{
  int i = 0;

  int num_words = words.size();

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

  int num_words = words.size();

  while (*i < num_words) {
    const string &word = words[*i].getWord();

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

  int num_words = words.size();

  while (i < num_words) {
    CwshCmd *cmd = new CwshCmd();

    wordsToCommand(words, &i, cmd);

    cmds.push_back(cmd);
  }

  //------

  if (cmds.size() > 0) {
    CwshCmd *last_cmd = cmds[cmds.size() - 1];

    CwshCmdSeparatorType separator_type = last_cmd->getSeparator().getType();

    if (separator_type == CWSH_COMMAND_SEPARATOR_PIPE     ||
        separator_type == CWSH_COMMAND_SEPARATOR_PIPE_ERR ||
        separator_type == CWSH_COMMAND_SEPARATOR_AND      ||
        separator_type == CWSH_COMMAND_SEPARATOR_OR)
      CWSH_THROW("Invalid null command.");
  }

  return true;
}

void
CwshCmdSplit::
wordsToCommand(const CwshWordArray &words, int *i, CwshCmd *cmd)
{
  int brackets = 0;

  int num_words = words.size();

  while (*i < num_words) {
    const string &word = words[*i].getWord();

    if      (word == "(")
      ++brackets;
    else if (word == ")") {
      --brackets;

      if (brackets < 0)
        CWSH_THROW("Too many )'s.");
    }
    else if (brackets == 0) {
      CwshCmdSeparator separator = parseCommandSeparator(word);

      if (separator.getType() != CWSH_COMMAND_SEPARATOR_NONE) {
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
parseCommandSeparator(const string &word)
{
  if      (word == "&")
    return CwshCmdSeparator(CWSH_COMMAND_SEPARATOR_BACKGROUND);
  else if (word == "|")
    return CwshCmdSeparator(CWSH_COMMAND_SEPARATOR_PIPE);
  else if (word == "|&")
    return CwshCmdSeparator(CWSH_COMMAND_SEPARATOR_PIPE_ERR);
  else if (word == "&&")
    return CwshCmdSeparator(CWSH_COMMAND_SEPARATOR_AND);
  else if (word == "||")
    return CwshCmdSeparator(CWSH_COMMAND_SEPARATOR_OR);
  else if (word == ";")
    return CwshCmdSeparator(CWSH_COMMAND_SEPARATOR_NORMAL);
  else
    return CwshCmdSeparator(CWSH_COMMAND_SEPARATOR_NONE);
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
  for_each(commands_.begin(), commands_.end(), CDeletePointer());
}

void
CwshCmd::
displayCmdArray(const CwshCmdArray &cmds)
{
  for_each(cmds.begin(), cmds.end(), &CwshCmd::displayCmd);
}

void
CwshCmd::
displayCmd(const CwshCmd *cmd)
{
  cmd->display();
}

CwshCmd::
CwshCmd() :
 separator_(CWSH_COMMAND_SEPARATOR_NONE)
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
  string command_str = CwshWord::toString(words_);

  cerr << command_str << " " << separator_.getName() << endl;
}

string
CwshCmdSeparator::
getName() const
{
  if      (type_ == CWSH_COMMAND_SEPARATOR_BACKGROUND)
    return "&";
  else if (type_ == CWSH_COMMAND_SEPARATOR_PIPE)
    return "|";
  else if (type_ == CWSH_COMMAND_SEPARATOR_PIPE_ERR)
    return "|&";
  else if (type_ == CWSH_COMMAND_SEPARATOR_AND)
    return "&&";
  else if (type_ == CWSH_COMMAND_SEPARATOR_OR)
    return "||";
  else
    return "";
}
