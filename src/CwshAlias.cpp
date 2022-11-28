#include <CwshI.h>
#include <CwshHistoryParser.h>

namespace Cwsh {

//---

AliasMgr::
AliasMgr(App *cwsh) :
 cwsh_(cwsh)
{
}

AliasMgr::
~AliasMgr()
{
}

Alias *
AliasMgr::
define(const std::string &name, const std::string &value)
{
  auto alias = std::make_shared<Alias>(name, value);

  aliases_[name] = alias;

  return alias.get();
}

void
AliasMgr::
undefine(const std::string &name)
{
  aliases_.erase(name);
}

Alias *
AliasMgr::
lookup(const std::string &name) const
{
  auto p = aliases_.find(name);

  return (p != aliases_.end() ? (*p).second.get() : nullptr);
}

bool
AliasMgr::
substitute(Cmd *cmd, CmdArray &cmds) const
{
  const auto &word = cmd->getWord(0);

  //------

  const std::string &str = word.getWord();

  if (str.empty() || str[0] == '"' || str[0] == '\'' || str[0] == '`' || str[0] == '\\')
    return false;

  //------

  Alias *alias = lookup(str);

  if (! alias || alias == lastAlias_)
    return false;

  //------

  HistoryParser parser(cwsh_);

  parser.parse(alias->getValue());

  if (cwsh_->getDebug())
    parser.display();

  std::vector<std::string> words;

  for (const auto &cword : cmd->getWords())
    words.push_back(cword.getWord());

  std::string line = parser.apply(words);

  //------

  WordArray words1;

  Word::toWords(line, words1);

  if (cwsh_->getDebug()) {
    std::cerr << "Split String Into Words\n";

    Word::printWords(words1);
  }

  //------

  auto *th = const_cast<AliasMgr *>(this);

  th->lastAlias_ = alias;

  CmdSplit::wordsToCommands(words1, cmds);

  int num_cmds = int(cmds.size());

  if (num_cmds > 0)
    cmds[num_cmds - 1]->setSeparator(cmd->getSeparator());

  if (cwsh_->getDebug()) {
    std::cerr << "Substitute Alias\n";

    Cmd::displayCmdArray(cmds);
  }

  th->lastAlias_ = nullptr;

  //------

  return true;
}

void
AliasMgr::
display(bool all) const
{
  for (auto &alias : aliases_)
    alias.second->display(all);
}

std::string
AliasMgr::
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

Alias::
Alias(const std::string &name, const std::string &value) :
 name_(name), value_(value)
{
}

Alias::
~Alias()
{
}

void
Alias::
display(bool all) const
{
  auto *mgr = CwshMgrInst;

  std::cout << mgr->aliasNameColorStr() << getName() << mgr->resetColorStr() << " ";

  displayValue(all);
}

void
Alias::
displayValue(bool all) const
{
  auto *mgr = CwshMgrInst;

  std::cout << mgr->aliasValueColorStr() << getValue() << mgr->resetColorStr();

  if (all) {
    std::cout << " [";

    std::cout << mgr->locationColorStr() << getFilename() << ":" << getLineNum() <<
                 mgr->resetColorStr();

    std::cout << "]";
  }

  std::cout << "\n";
}

//---

}
