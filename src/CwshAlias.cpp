#include "CwshI.h"
#include <CwshHistoryParser.h>

template<typename T>
class CwshAliasListValueDisplay {
 public:
  void operator()(const typename T::value_type &alias) {
    alias.second->display();
  }
};

CwshAliasMgr::
CwshAliasMgr(Cwsh *cwsh) :
 cwsh_(cwsh), last_alias_(NULL)
{
}

CwshAliasMgr::
~CwshAliasMgr()
{
}

void
CwshAliasMgr::
define(const CwshAliasName &name, const CwshAliasValue &value)
{
  CwshAlias *alias = new CwshAlias(name, value);

  aliases_.setValue(name, alias);
}

void
CwshAliasMgr::
undefine(const CwshAliasName &name)
{
  aliases_.unsetValue(name);
}

CwshAlias *
CwshAliasMgr::
lookup(const string &name) const
{
  return aliases_.getValue(name);
}

bool
CwshAliasMgr::
substitute(CwshCmd *cmd, CwshCmdArray &cmds) const
{
  const CwshWord &word = cmd->getWord(0);

  //------

  const string &str = word.getWord();

  if (str.empty() || str[0] == '"' || str[0] == '\'' || str[0] == '`' || str[0] == '\\')
    return false;

  //------

  CwshAlias *alias = lookup(str);

  if (alias == NULL || alias == last_alias_)
    return false;

  //------

  CwshHistoryParser parser(cwsh_);

  parser.parse(alias->getValue());

  if (cwsh_->getDebug())
    parser.display();

  vector<string> words;

  int num_words = cmd->getNumWords();

  for (int i = 0; i < num_words; i++)
    words.push_back(cmd->getWord(i).getWord());

  string line = parser.apply(words);

  //------

  CwshWordArray words1;

  CwshWord::toWords(line, words1);

  if (cwsh_->getDebug()) {
    cerr << "Split String Into Words" << endl;

    CwshWord::printWords(words1);
  }

  //------

  CwshAliasMgr *th = const_cast<CwshAliasMgr *>(this);

  th->last_alias_ = alias;

  CwshCmdSplit::wordsToCommands(words1, cmds);

  int num_cmds = cmds.size();

  if (num_cmds > 0)
    cmds[num_cmds - 1]->setSeparator(cmd->getSeparator());

  if (cwsh_->getDebug()) {
    cerr << "Substitute Alias" << endl;

    CwshCmd::displayCmdArray(cmds);
  }

  th->last_alias_ = NULL;

  //------

  return true;
}

void
CwshAliasMgr::
display() const
{
  for_each(aliases_.begin(), aliases_.end(), CwshAliasListValueDisplay<AliasList>());
}

string
CwshAliasMgr::
getAliasesMsg() const
{
  string msg;

  AliasList::const_iterator palias1 = aliases_.begin();
  AliasList::const_iterator palias2 = aliases_.end  ();

  for ( ; palias1 != palias2; ++palias1) {
    if (! msg.empty()) msg += "#";

    msg += (*palias1).second->getName () + "#" + (*palias1).second->getValue();
  }

  return msg;
}

//-------------------

CwshAlias::
CwshAlias(const string &name, const string &value) :
 name_(name), value_(value)
{
}

CwshAlias::
~CwshAlias()
{
}

void
CwshAlias::
display() const
{
  cout << name_ << " " << value_ << endl;
}
