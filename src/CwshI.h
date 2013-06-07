#include <CFile.h>
#include <CDir.h>
#include <CTempFile.h>
#include <CStrUtil.h>
#include <CInstCount.h>
#include <CFuncs.h>
#include <CThrow.h>
#include <COSFile.h>
#include <COSUser.h>
#include <COSEnv.h>

#include "CwshTypes.h"

#include "CwshWord.h"

#include "Cwsh.h"
#include "CwshAlias.h"
#include "CwshAutoExec.h"
#include "CwshBlock.h"
#include "CwshBraces.h"
#include "CwshCmdSplit.h"
#include "CwshCommand.h"
#include "CwshComplete.h"
#include "CwshDir.h"
#include "CwshDirStack.h"
#include "CwshExprEvaluate.h"
#include "CwshExprOperator.h"
#include "CwshExprParse.h"
#include "CwshExprProcess.h"
#include "CwshExprStack.h"
#include "CwshFunction.h"
#include "CwshHash.h"
#include "CwshHistory.h"
#include "CwshInPlaceCommand.h"
#include "CwshInput.h"
#include "CwshMatch.h"
#include "CwshPattern.h"
#include "CwshProcess.h"
#include "CwshReadLine.h"
#include "CwshResource.h"
#include "CwshServer.h"
#include "CwshShellCommand.h"
#include "CwshSignal.h"
#include "CwshState.h"
#include "CwshString.h"
#include "CwshUnixCommand.h"
#include "CwshVariable.h"
#include "CwshVariableParser.h"
#include "CwshWildCard.h"

#include "CwshSet.h"

#include "CwshErr.h"

#include <algorithm>

using std::string;
using std::vector;
using std::list;
using std::set;
using std::ostream;
using std::cout;
using std::cerr;
using std::endl;
