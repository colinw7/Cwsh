class CwshInput {
 public:
  CwshInput(Cwsh *cwsh);
 ~CwshInput();

  void execute(const std::string &filename);
  void execute(CFile *file);

  void processLine(const CwshLine &line);

  void executeCommands(const CwshCmdArray &cmds);

  void getBlock(CwshShellCommand *command, CwshLineArray &lines);
  void skipBlock(const CwshLine &line);

  bool     eof();
  CwshLine getLine();

  std::string getPrompt();

  std::string processExprLine(const CwshLine &line);

 private:
  void executeLine(std::string &line);
  void executeLines(std::vector<std::string> &lines);
  void executeLines(bool interactive);

  void executeFile();
  void executeStdIn();

  std::string readStdInToken(const std::string &token);
  std::string processStdInLine(const CwshLine &line);

 private:
  CPtr<Cwsh>       cwsh_;
  bool             history_active_;
  CFile           *input_file_;
  CwshCommandData *current_command_;
};
