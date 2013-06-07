class CwshString {
 public:
  static std::string replaceHome(const std::string &str);
  static void        skipWordsToChar(const std::string &str, uint *i, int c);
  static void        skipWord(const std::string &str, uint *i);
  static void        addWords(const std::string &line, std::vector<std::string> &words);
  static std::string readLineFromFile(CFile *file);

  static bool matchUsers(const std::string &pattern, std::vector<std::string> &names);
};
