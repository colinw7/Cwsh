class CwshInput {
 public:
  CwshInput(Cwsh *cwsh);
 ~CwshInput();

  bool execute(const std::string &filename);
  bool execute(CFile *file);

  void processLine(const CwshLine &line);

  void executeCommands(const CwshCmdArray &cmds);

  void getBlock(CwshShellCommand *command, CwshLineArray &lines);
  void skipBlock(const CwshLine &line);

  bool eof();

  std::string getFilename() const;

  CwshLine getLine();

  std::string getPrompt();

  std::string processExprLine(const CwshLine &line);

 private:
  bool executeFile(CFile *file);

  void executeLine(std::string &line);
  void executeLines(const CwshLineArray &lines);
  void executeBlockLines(bool interactive);

  bool executeCurrentFile();
  bool executeStdIn();

  std::string readStdInToken(const std::string &token);
  std::string processStdInLine(const CwshLine &line);

 private:
  CPtr<Cwsh>       cwsh_;
  bool             historyActive_  { false };
  CFile           *inputFile_      { nullptr };
  CwshCommandData *currentCommand_ { nullptr };
};
