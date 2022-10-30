enum class CwshBlockType {
  FOREACH,
  FILE,
  FUNCTION,
  IF,
  SWITCH,
  WHILE
};

class  Cwsh;
class  CwshAlias;
class  CwshAliasMgr;
class  CwshAutoExec;
class  CwshAutoExecMgr;
class  CwshBlock;
class  CwshBlockMgr;
class  CwshCommand;
class  CwshCommandData;
class  CwshCmd;
class  CwshCmdLine;
class  CwshCmdGroup;
class  CwshDirStack;
class  CwshExprStackStack;
class  CwshExprStack;
class  CwshExprStackNode;
class  CwshExprOperator;
class  CwshFunctionMgr;
class  CwshFunction;
class  CwshHash;
class  CwshHistory;
class  CwshHistoryCmdData;
class  CwshHistoryOperation;
class  CwshInput;
class  CwshProcessMgr;
class  CwshProcess;
class  CwshReadLine;
class  CwshResource;
struct CwshResourceLimit;
class  CwshServer;
class  CwshShellCommandMgr;
class  CwshShellCommand;
class  CwshState;
class  CwshStateMgr;
class  CwshVariable;
class  CwshVariableMgr;
class  CwshWord;
class  CwshSubWord;

#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>

typedef std::string           CwshArg;
typedef std::vector<CwshArg>  CwshArgArray;

//---

struct CwshLine {
  std::string line;
  int         num { -1 };

  CwshLine(const std::string &line_, int num_=-1) :
   line(line_), num(num_) {
  }
};

typedef std::vector<CwshLine> CwshLineArray;

//---

typedef std::string                         CwshAliasName;
typedef std::string                         CwshAliasValue;
typedef std::string                         CwshAutoExecName;
typedef std::string                         CwshAutoExecValue;
typedef std::vector<CwshBlock *>            CwshBlockArray;
typedef std::vector<CwshCmd *>              CwshCmdArray;
typedef std::vector<CwshCmdLine *>          CwshCmdLineArray;
typedef std::vector<CwshCmdGroup *>         CwshCmdGroupArray;
typedef std::vector<CwshExprStack *>        CwshExprStackArray;
typedef std::list<CwshExprStackNode *>      CwshExprStackNodeList;
typedef std::string                         CwshFunctionName;
typedef std::map<std::string,std::string>   CwshHashFilePathMap;
typedef std::vector<CwshHistoryOperation *> CwshHistoryOperationArray;
typedef std::list<CwshVariable *>           CwshVariableList;
typedef std::vector<CwshVariableMgr *>      CwshVariableMgrArray;
typedef std::string                         CwshVariableName;
typedef std::string                         CwshVariableValue;
typedef std::vector<CwshVariableValue>      CwshVariableValueArray;
typedef std::vector<CwshWord>               CwshWordArray;
typedef std::vector<CwshSubWord>            CwshSubWordArray;
typedef std::list<CwshHistoryCmdData *>     CwshHistoryCmdDataList;

typedef void (*CwshShellCommandProc)(Cwsh *cwsh, const CwshArgArray &args);
