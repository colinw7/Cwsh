enum class CwshVariableValueType {
  VALUE,
  SIZE,
  EXISTS
};

enum class CwshVariableValueModifier {
  NONE,
  ROOT,
  EXTENSION,
  HEADER,
  TAIL,
  QUOTE_WORDLIST,
  QUOTE_WORD
};

class CwshVariableParser {
 public:
  CwshVariableParser(Cwsh *cwsh, const CwshWord &word);

  bool expandVariables(CwshWordArray &words);

 private:
  std::string expandQuotedVariables(const CwshWord &str);

  bool expandVariables1(const std::string &str, std::vector<std::string> &words);
  bool expandVariable(const std::string &name, std::vector<std::string> &words);

 private:
  CPtr<Cwsh>      cwsh_;
  const CwshWord &word_;
};
