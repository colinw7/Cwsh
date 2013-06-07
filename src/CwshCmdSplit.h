#ifndef CWSH_CMD_SPLIT_H
#define CWSH_CMD_SPLIT_H

enum CwshCmdSeparatorType {
  CWSH_COMMAND_SEPARATOR_NONE,
  CWSH_COMMAND_SEPARATOR_BACKGROUND,
  CWSH_COMMAND_SEPARATOR_PIPE,
  CWSH_COMMAND_SEPARATOR_PIPE_ERR,
  CWSH_COMMAND_SEPARATOR_AND,
  CWSH_COMMAND_SEPARATOR_OR,
  CWSH_COMMAND_SEPARATOR_NORMAL
};

class CwshCmdSeparator {
 public:
  CwshCmdSeparator(CwshCmdSeparatorType type) : type_(type) { }

  CwshCmdSeparatorType getType() const { return type_; }
  std::string          getName() const;

 private:
  CwshCmdSeparatorType type_;
};

class CwshCmdLine {
  CINST_COUNT_MEMBER(CwshCmdLine);

 public:
  CwshCmdLine();
 ~CwshCmdLine();

  const CwshWordArray &getWords() const { return words_; }

  int             getNumWords() const { return words_.size(); }
  const CwshWord &getWord(int i) const { return words_[i]; }

  void addWord(const CwshWord &word);

 private:
  CwshWordArray words_;
};

class CwshCmdGroup {
  CINST_COUNT_MEMBER(CwshCmdGroup);

 public:
  CwshCmdGroup(const CwshCmdArray &commands);
 ~CwshCmdGroup();

  const CwshCmdArray &getCommands() const { return commands_; }

  int            getNumCommands() const { return commands_.size(); }
  const CwshCmd *getCommand(int i) const { return commands_[i]; }

 private:
  CwshCmdArray commands_;
};

class CwshCmdStdIn {
 public:
  CwshCmdStdIn() : has_token_(false), filename_("") { }

  void setFile (const std::string &file ) { has_token_ = false; filename_ = file ; }
  void setToken(const std::string &token) { has_token_ = true ; filename_ = token; }

  bool hasFile () const { return (! has_token_ && filename_ != ""); }
  bool hasToken() const { return has_token_; }

  const std::string &getFile () const { return filename_; }
  const std::string &getToken() const { return filename_; }

 private:
  bool        has_token_;
  std::string filename_;
};

class CwshCmdStdOut {
 public:
  CwshCmdStdOut() : filename_(""), clobber_(false), append_(false) { }

  void setFile   (const std::string &file) { filename_    = file; }
  void setClobber(bool flag=true    ) { clobber_  = flag; }
  void setAppend (bool flag=true    ) { append_   = flag; }

  bool hasFile() const { return filename_ != ""; }

  const std::string &getFile   () const { return filename_   ; }
  bool          getClobber() const { return clobber_ ; }
  bool          getAppend () const { return append_  ; }

 private:
  std::string filename_;
  bool        clobber_;
  bool        append_;
};

class CwshCmd {
  CINST_COUNT_MEMBER(CwshCmd);

 public:
  static void displayCmdArray(const CwshCmdArray &cmds);
  static void displayCmd(const CwshCmd *cmd);

  CwshCmd();
 ~CwshCmd();

  int             getNumWords() const { return words_.size(); }
  const CwshWord &getWord(int i) const { return words_[i]; }

  const CwshCmdSeparator &getSeparator() const { return separator_; }

  void addWord(const CwshWord &word);

  void setWord(int i, const CwshWord &word);
  void setWords(const CwshWordArray &words);
  void setSeparator(const CwshCmdSeparator &separator);

  void setStdInFile (const std::string &file ) { stdin_.setFile (file ); }
  void setStdInToken(const std::string &token) { stdin_.setToken(token); }

  bool hasStdInFile () const { return stdin_.hasFile (); }
  bool hasStdInToken() const { return stdin_.hasToken(); }

  const std::string &getStdInFile () const { return stdin_.getFile (); }
  const std::string &getStdInToken() const { return stdin_.getToken(); }

  void setStdOutFile   (const std::string &file) { stdout_.setFile   (file); }
  void setStdOutClobber(bool flag=true         ) { stdout_.setClobber(flag); }
  void setStdOutAppend (bool flag=true         ) { stdout_.setAppend (flag); }

  bool hasStdOutFile() const { return stdout_.hasFile(); }

  const std::string &getStdOutFile   () const { return stdout_.getFile   (); }
  bool               getStdOutClobber() const { return stdout_.getClobber(); }
  bool               getStdOutAppend () const { return stdout_.getAppend (); }

  void setStdErrFile   (const std::string &file) { stderr_.setFile   (file); }
  void setStdErrClobber(bool flag=true         ) { stderr_.setClobber(flag); }
  void setStdErrAppend (bool flag=true         ) { stderr_.setAppend (flag); }

  bool hasStdErrFile() const { return stderr_.hasFile(); }

  const std::string &getStdErrFile   () const { return stderr_.getFile   (); }
  bool               getStdErrClobber() const { return stderr_.getClobber(); }
  bool               getStdErrAppend () const { return stderr_.getAppend (); }

  void display() const;

 private:
  CwshWordArray    words_;
  CwshCmdSeparator separator_;
  CwshCmdStdIn     stdin_;
  CwshCmdStdOut    stdout_;
  CwshCmdStdOut    stderr_;
};

class CwshCmdSplit {
 public:
  static bool wordsToCommandLines(const CwshWordArray &words, CwshCmdLineArray &cmds);

  static bool wordsToCommands(const CwshWordArray &words, CwshCmdArray &cmds);

 private:
  static void wordsToCommandLine(const CwshWordArray &words, int *i, CwshCmdLine *cmd);

  static void wordsToCommand(const CwshWordArray &words, int *i, CwshCmd *cmd);

  static CwshCmdSeparator parseCommandSeparator(const std::string &word);
};

#endif
