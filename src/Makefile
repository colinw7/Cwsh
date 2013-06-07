CC = g++
RM = rm

CDEBUG = -g
LDEBUG = -g

INC_DIR = ../include
OBJ_DIR = ../obj
LIB_DIR = ../lib
BIN_DIR = ../bin

all: $(BIN_DIR)/Cwsh

clean:
	$(RM) -f $(OBJ_DIR)/*.o
	$(RM) -f $(BIN_DIR)/Cwsh

SRC = \
CwshAlias.cpp \
CwshAutoExec.cpp \
CwshBlock.cpp \
CwshBraces.cpp \
CwshCmdSplit.cpp \
CwshCommand.cpp \
CwshComplete.cpp \
Cwsh.cpp \
CwshDir.cpp \
CwshDirStack.cpp \
CwshExprEvaluate.cpp \
CwshExprOperator.cpp \
CwshExprParse.cpp \
CwshExprProcess.cpp \
CwshExprStack.cpp \
CwshFunction.cpp \
CwshHash.cpp \
CwshHistory.cpp \
CwshHistoryParser.cpp \
CwshInPlaceCommand.cpp \
CwshInput.cpp \
CwshMain.cpp \
CwshMatch.cpp \
CwshPattern.cpp \
CwshProcess.cpp \
CwshReadLine.cpp \
CwshResource.cpp \
CwshServer.cpp \
CwshSet.cpp \
CwshShellCommand.cpp \
CwshSignal.cpp \
CwshState.cpp \
CwshString.cpp \
CwshUnixCommand.cpp \
CwshVariable.cpp \
CwshVariableParser.cpp \
CwshWildCard.cpp \
CwshWord.cpp \
CCommand.cpp \
CCommandDest.cpp \
CCommandFileDest.cpp \
CCommandFileSrc.cpp \
CCommandPipe.cpp \
CCommandPipeDest.cpp \
CCommandPipeSrc.cpp \
CCommandSrc.cpp \
CCommandStringDest.cpp \
CCommandStringSrc.cpp \
CCommandUtil.cpp \
CRGBName.cpp \
CPathList.cpp \
CFileMatch.cpp \
CReadLine.cpp \
CHistory.cpp \
COSLimit.cpp \
COSTerm.cpp \
COSErr.cpp \
CEscape.cpp \

OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC))

CPPFLAGS = \
-I$(INC_DIR) \
-I../../CFile/include \
-I../../CArgs/include \
-I../../CStrUtil/include \
-I../../CGlob/include \
-I../../COS/include \
-I../../CUtil/include \
-I.

LFLAGS = \
$(LEBUG) \
-L$(LIB_DIR) \
-L../../CFile/lib \
-L../../CArgs/lib \
-L../../CStrUtil/lib \
-L../../CGlob/lib \
-L../../COS/lib \
-lCFile \
-lCArgs \
-lCOS \
-lCGlob \
-lCStrUtil \
-lreadline

.SUFFIXES: .cpp

$(OBJS): $(OBJ_DIR)/%.o: %.cpp
	$(CC) -c $< -o $(OBJ_DIR)/$*.o $(CPPFLAGS)

$(BIN_DIR)/Cwsh: $(OBJS)
	$(CC) -o $(BIN_DIR)/Cwsh $(OBJS) $(LFLAGS)
