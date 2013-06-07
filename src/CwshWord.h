enum CwshSubWordType {
  CWSH_SUB_WORD_TYPE_DOUBLE_QUOTED,
  CWSH_SUB_WORD_TYPE_SINGLE_QUOTED,
  CWSH_SUB_WORD_TYPE_BACK_QUOTED,
  CWSH_SUB_WORD_TYPE_NORMAL,
};

class CwshWord {
 private:
  std::string      word_;
  bool             sub_words_created_;
  CwshSubWordArray sub_words_;

 public:
  static void toWords(const std::string &line, CwshWordArray &words);

  static std::string toString(const CwshWordArray &words);
  static std::string toString(const CwshSubWordArray &sub_words);

  static void printWords(const CwshWordArray &words);

  static void printWord(const CwshWord &word);

  CwshWord(const std::string &word="");

  const std::string &getWord() const { return word_; }

  const CwshSubWordArray &getSubWords() const;

  void removeQuotes();

  friend std::ostream &operator<<(std::ostream &os, const CwshWord &word);

 private:
  void createSubWords();
};

class CwshSubWord {
 private:
  std::string     word_;
  CwshSubWordType type_;

 public:
  static std::string toString(const CwshSubWordArray &words);

  CwshSubWord(const std::string &word, CwshSubWordType type=CWSH_SUB_WORD_TYPE_NORMAL);

  const std::string &getWord() const { return word_; }
  CwshSubWordType    getType() const { return type_; }

  std::string getString() const;

  friend std::ostream &operator<<(std::ostream &os, const CwshSubWord &word);
};
