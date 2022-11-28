namespace Cwsh {

class Input {
 public:
  Input(App *cwsh);
 ~Input();

  bool execute(const std::string &filename);
  bool execute(CFile *file);

  void processLine(const Line &line);

  void executeCommands(const CmdArray &cmds);

  void getBlock(ShellCommand *command, LineArray &lines);
  void skipBlock(const Line &line);

  bool eof();

  std::string getFilename() const;

  Line getLine();

  std::string getPrompt();

  std::string processExprLine(const Line &line);

 private:
  bool executeFile(CFile *file);

  void executeLine(std::string &line);
  void executeLines(const LineArray &lines);
  void executeBlockLines(bool interactive);

  bool executeCurrentFile();
  bool executeStdIn();

  std::string readStdInToken(const std::string &token);
  std::string processStdInLine(const Line &line);

 private:
  CPtr<App>    cwsh_;
  bool         historyActive_  { false };
  CFile       *inputFile_      { nullptr };
  CommandData *currentCommand_ { nullptr };
};

}
