namespace Cwsh {

enum class BlockType {
  FOREACH,
  FILE,
  FUNCTION,
  IF,
  SWITCH,
  WHILE
};

}

//---

#include <memory>
#include <map>
#include <list>
#include <vector>

//---

namespace Cwsh {

class hApp;

class AliasMgr;
class Alias;

using AliasP    = std::shared_ptr<Alias>;
using AliasMgrP = std::unique_ptr<AliasMgr>;

class AutoExecMgr;
class AutoExec;

using AutoExecP    = std::shared_ptr<AutoExec>;
using AutoExecMgrP = std::unique_ptr<AutoExecMgr>;

class BlockMgr;
class Block;

using BlockMgrP = std::unique_ptr<BlockMgr>;
using BlockP    = std::shared_ptr<Block>;

using BlockArray = std::vector<BlockP>;

class Cmd;
class CmdLine;
class CmdGroup;

using CmdArray      = std::vector<Cmd *>;
using CmdLineArray  = std::vector<CmdLine *>;
using CmdGroupP     = std::shared_ptr<CmdGroup>;
using CmdGroupArray = std::vector<CmdGroupP>;

class Command;
class CommandData;

class DirStack;

using DirStackP = std::unique_ptr<DirStack>;

class ExprStackStack;
class ExprStack;
class ExprStackNode;
class ExprOperator;

using ExprStackP        = std::shared_ptr<ExprStack>;
using ExprStackArray    = std::vector<ExprStackP>;
using ExprStackNodeList = std::list<ExprStackNode *>;

class FunctionMgr;
class Function;

using FunctionMgrP = std::unique_ptr<FunctionMgr>;

class Hash;

using HashP = std::unique_ptr<Hash>;

using HashFilePathMap = std::map<std::string, std::string>;

class History;
class HistoryCmdData;
class HistoryOperation;

using HistoryP = std::unique_ptr<History>;

using HistoryCmdDataList = std::list<HistoryCmdData *>;

using HistoryOperationArray = std::vector<HistoryOperation *>;

class Input;

using InputP = std::unique_ptr<Input>;

class ProcessMgr;
class Process;

using ProcessMgrP = std::unique_ptr<ProcessMgr>;

class ReadLine;

using ReadLineP = std::unique_ptr<ReadLine>;

class  Resource;
struct ResourceLimit;

using ResourceP = std::unique_ptr<Resource>;

class Server;

using ServerP = std::unique_ptr<Server>;

class ShellCommandMgr;
class ShellCommand;

using ShellCommandMgrP = std::unique_ptr<ShellCommandMgr>;

#ifdef USE_SHM
class ShMem;

using ShMemP = std::unique_ptr<ShMem>;
#endif

class StateMgr;
class State;

using StateMgrP = std::unique_ptr<StateMgr>;

class VariableMgr;
class Variable;

using VariableMgrP = std::unique_ptr<VariableMgr>;

using VariableMgrArray = std::vector<VariableMgr *>;

using VariableList = std::list<Variable *>;

using VariableValueArray = std::vector<std::string>;

class Word;
class SubWord;

using WordArray    = std::vector<Word>;
using SubWordArray = std::vector<SubWord>;

}

//---

#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>

//---

namespace Cwsh {

using ArgArray = std::vector<std::string>;

//---

struct Line {
  std::string line;
  int         num { -1 };

  Line(const std::string &line_, int num_=-1) :
   line(line_), num(num_) {
  }
};

using LineArray = std::vector<Line>;

}
