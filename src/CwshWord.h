namespace Cwsh {

enum class SubWordType {
  DOUBLE_QUOTED,
  SINGLE_QUOTED,
  BACK_QUOTED,
  NORMAL
};

class Word {
 public:
  static void toWords(const std::string &line, WordArray &words);

  static std::string toString(const WordArray &words);
  static std::string toString(const SubWordArray &subWords);

  static void printWords(const WordArray &words);

  static void printWord(const Word &word);

  explicit Word(const std::string &word="");

  const std::string &getWord() const { return word_; }

  const SubWordArray &getSubWords() const;

  void removeQuotes();

  friend std::ostream &operator<<(std::ostream &os, const Word &word);

 private:
  void createSubWords();

 private:
  std::string  word_;
  bool         subWordsCreated_ { false };
  SubWordArray subWords_;
};

//---

class SubWord {
 public:
  static std::string toString(const SubWordArray &words);

  explicit SubWord(const std::string &word, SubWordType type=SubWordType::NORMAL);

  const std::string &getWord() const { return word_; }
  SubWordType        getType() const { return type_; }

  std::string getString() const;

  friend std::ostream &operator<<(std::ostream &os, const SubWord &word);

 private:
  std::string word_;
  SubWordType type_;
};

}
