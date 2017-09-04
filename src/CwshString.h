namespace CwshString {
  std::string replaceHome(const std::string &str);
  void        skipWordsToChar(const std::string &str, uint *i, int c);
  void        skipWord(const std::string &str, uint *i);
  void        addWords(const std::string &line, std::vector<std::string> &words);
  std::string readLineFromFile(CFile *file);

  bool matchUsers(const std::string &pattern, std::vector<std::string> &names);
}
