#include <CwshI.h>
#include <CwshHistoryParser.h>

CwshAliasMgr::
CwshAliasMgr(Cwsh *cwsh) :
 cwsh_(cwsh)
{
}

CwshAliasMgr::
~CwshAliasMgr()
{
}

CwshAlias *
CwshAliasMgr::
define(const CwshAliasName &name, const CwshAliasValue &value)
{
  CwshAlias *alias = new CwshAlias(name, value);

  aliases_.setValue(name, alias);

  return alias;
}

void
CwshAliasMgr::
undefine(const CwshAliasName &name)
{
  aliases_.unsetValue(name);
}

CwshAlias *
CwshAliasMgr::
lookup(const std::string &name) const
{
  return aliases_.getValue(name);
}

bool
CwshAliasMgr::
substitute(CwshCmd *cmd, CwshCmdArray &cmds) const
{
  const CwshWord &word = cmd->getWord(0);

  //------

  const std::string &str = word.getWord();

  if (str.empty() || str[0] == '"' || str[0] == '\'' || str[0] == '`' || str[0] == '\\')
    return false;

  //------

  CwshAlias *alias = lookup(str);

  if (! alias || alias == last_alias_)
    return false;

  //------

  CwshHistoryParser parser(cwsh_);

  parser.parse(alias->getValue());

  if (cwsh_->getDebug())
    parser.display();

  std::vector<std::string> words;

  for (const auto &cword : cmd->getWords())
    words.push_back(cword.getWord());

  std::string line = parser.apply(words);

  //------

  CwshWordArray words1;

  CwshWord::toWords(line, words1);

  if (cwsh_->getDebug()) {
    std::cerr << "Split String Into Words" << std::endl;

    CwshWord::printWords(words1);
  }

  //------

  CwshAliasMgr *th = const_cast<CwshAliasMgr *>(this);

  th->last_alias_ = alias;

  CwshCmdSplit::wordsToCommands(words1, cmds);

  int num_cmds = int(cmds.size());

  if (num_cmds > 0)
    cmds[num_cmds - 1]->setSeparator(cmd->getSeparator());

  if (cwsh_->getDebug()) {
    std::cerr << "Substitute Alias" << std::endl;

    CwshCmd::displayCmdArray(cmds);
  }

  th->last_alias_ = nullptr;

  //------

  return true;
}

void
CwshAliasMgr::
display(bool all) const
{
  for (auto &alias : aliases_)
    alias.second->display(all);
}

std::string
CwshAliasMgr::
getAliasesMsg() const
{
  std::string msg;

  for (const auto &alias : aliases_) {
    if (! msg.empty())
      msg += "#";

    msg += alias.second->getName() + "#" + alias.second->getValue();
  }

  return msg;
}

//-------------------

CwshAlias::
CwshAlias(const std::string &name, const std::string &value) :
 name_(name), value_(value)
{
}

CwshAlias::
~CwshAlias()
{
}

void
CwshAlias::
display(bool all) const
{
  std::cout << CwshMgrInst.aliasNameColorStr() << getName() <<
               CwshMgrInst.resetColorStr() << " ";

  displayValue(all);
}

void
CwshAlias::
displayValue(bool all) const
{
  std::cout << CwshMgrInst.aliasValueColorStr() << getValue() <<
               CwshMgrInst.resetColorStr();

  if (all) {
    std::cout << " [";

    std::cout << CwshMgrInst.locationColorStr() << getFilename() << ":" << getLineNum() <<
                 CwshMgrInst.resetColorStr();

    std::cout << "]";
  }

  std::cout << std::endl;
}
