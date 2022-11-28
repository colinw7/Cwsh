#ifndef CWSH_CMD_SPLIT_H
#define CWSH_CMD_SPLIT_H

namespace Cwsh {

enum class CmdSeparatorType {
  NONE,
  BACKGROUND,
  PIPE,
  PIPE_ERR,
  AND,
  OR,
  NORMAL
};

class CmdSeparator {
 public:
  CmdSeparator(CmdSeparatorType type) : type_(type) { }

  CmdSeparatorType getType() const { return type_; }
  std::string      getName() const;

 private:
  CmdSeparatorType type_;
};

//---

class CmdLine {
  CINST_COUNT_MEMBER(CmdLine);

 public:
  CmdLine();
 ~CmdLine();

  const WordArray &getWords() const { return words_; }

  int         getNumWords() const { return int(words_.size()); }
  const Word &getWord(int i) const { return words_[uint(i)]; }

  void addWord(const Word &word);

 private:
  WordArray words_;
};

//---

class CmdGroup {
  CINST_COUNT_MEMBER(CmdGroup);

 public:
  CmdGroup(const CmdArray &commands);
 ~CmdGroup();

  const CmdArray &getCommands() const { return commands_; }

  int        getNumCommands() const { return int(commands_.size()); }
  const Cmd *getCommand(int i) const { return commands_[uint(i)]; }

 private:
  CmdArray commands_;
};

//---

class CmdStdIn {
 public:
  CmdStdIn() { }

  void setFile (const std::string &file ) { hasToken_ = false; filename_ = file ; }
  void setToken(const std::string &token) { hasToken_ = true ; filename_ = token; }

  bool hasFile () const { return (! hasToken_ && filename_ != ""); }
  bool hasToken() const { return hasToken_; }

  const std::string &getFile () const { return filename_; }
  const std::string &getToken() const { return filename_; }

 private:
  bool        hasToken_ { false };
  std::string filename_;
};

//---

class CmdStdOut {
 public:
  CmdStdOut() { }

  void setFile   (const std::string &file) { filename_    = file; }
  void setClobber(bool flag=true    ) { clobber_  = flag; }
  void setAppend (bool flag=true    ) { append_   = flag; }

  bool hasFile() const { return filename_ != ""; }

  const std::string &getFile   () const { return filename_   ; }
  bool          getClobber() const { return clobber_ ; }
  bool          getAppend () const { return append_  ; }

 private:
  std::string filename_;
  bool        clobber_ { false };
  bool        append_  { false };
};

//---

class Cmd {
  CINST_COUNT_MEMBER(Cmd);

 public:
  static void displayCmdArray(const CmdArray &cmds);
  static void displayCmd(const Cmd *cmd);

  Cmd();
 ~Cmd();

  int         getNumWords() const { return int(words_.size()); }
  const Word &getWord(int i) const { return words_[uint(i)]; }

  const CmdSeparator &getSeparator() const { return separator_; }

  const WordArray &getWords() const { return words_; }
  void setWords(const WordArray &words);

  void addWord(const Word &word);
  void setWord(int i, const Word &word);

  void setSeparator(const CmdSeparator &separator);

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
  WordArray    words_;
  CmdSeparator separator_;
  CmdStdIn     stdin_;
  CmdStdOut    stdout_;
  CmdStdOut    stderr_;
};

//---

class CmdSplit {
 public:
  static bool wordsToCommandLines(const WordArray &words, CmdLineArray &cmds);

  static bool wordsToCommands(const WordArray &words, CmdArray &cmds);

 private:
  static void wordsToCommandLine(const WordArray &words, int *i, CmdLine *cmd);

  static void wordsToCommand(const WordArray &words, int *i, Cmd *cmd);

  static CmdSeparator parseCommandSeparator(const std::string &word);
};

}

#endif
