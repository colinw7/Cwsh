namespace Cwsh {

enum class VariableValueType {
  VALUE,
  SIZE,
  EXISTS
};

enum class VariableValueModifier {
  NONE,
  ROOT,
  EXTENSION,
  HEADER,
  TAIL,
  QUOTE_WORDLIST,
  QUOTE_WORD
};

class VariableParser {
 public:
  VariableParser(App *cwsh, const Word &word);

  bool expandVariables(WordArray &words);

 private:
  std::string expandQuotedVariables(const Word &str);

  bool expandVariables1(const std::string &str, std::vector<std::string> &words);
  bool expandVariable(const std::string &name, std::vector<std::string> &words);

 private:
  CPtr<App>   cwsh_;
  const Word &word_;
};

}
