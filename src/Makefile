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
CPathList.cpp \
CFileMatch.cpp \
CEscape.cpp \

OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC))

CPPFLAGS = \
-std=c++14 \
-I$(INC_DIR) \
-I../../CCommand/include \
-I../../CFile/include \
-I../../CArgs/include \
-I../../CStrUtil/include \
-I../../CGlob/include \
-I../../COS/include \
-I../../CUtil/include \
-I../../CReadLine/include \
-I../../CRGBName/include \
-I../../CMath/include \
-I.

LFLAGS = \
$(LDEBUG) \
-L$(LIB_DIR) \
-L../../CCommand/lib \
-L../../CFile/lib \
-L../../CArgs/lib \
-L../../CStrUtil/lib \
-L../../CGlob/lib \
-L../../COS/lib \
-L../../CReadLine/lib \
-L../../CRGBName/lib \
-L../../CMath/lib \
-lCCommand \
-lCFile \
-lCArgs \
-lCReadLine \
-lCOS \
-lCGlob \
-lCRGBName \
-lCStrUtil \
-lreadline \
-lcurses

.SUFFIXES: .cpp

$(OBJS): $(OBJ_DIR)/%.o: %.cpp
	$(CC) -c $< -o $(OBJ_DIR)/$*.o $(CPPFLAGS)

$(BIN_DIR)/Cwsh: $(OBJS)
	$(CC) -o $(BIN_DIR)/Cwsh $(OBJS) $(LFLAGS)
