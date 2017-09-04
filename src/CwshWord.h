enum class CwshSubWordType {
  DOUBLE_QUOTED,
  SINGLE_QUOTED,
  BACK_QUOTED,
  NORMAL
};

class CwshWord {
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

 private:
  std::string      word_;
  bool             sub_words_created_ { false };
  CwshSubWordArray sub_words_;
};

//---

class CwshSubWord {
 public:
  static std::string toString(const CwshSubWordArray &words);

  CwshSubWord(const std::string &word, CwshSubWordType type=CwshSubWordType::NORMAL);

  const std::string &getWord() const { return word_; }
  CwshSubWordType    getType() const { return type_; }

  std::string getString() const;

  friend std::ostream &operator<<(std::ostream &os, const CwshSubWord &word);

 private:
  std::string     word_;
  CwshSubWordType type_;
};
