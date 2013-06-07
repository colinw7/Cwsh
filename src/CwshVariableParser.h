enum CwshVariableValueType {
  CWSH_VARIABLE_VALUE_TYPE_VALUE,
  CWSH_VARIABLE_VALUE_TYPE_SIZE,
  CWSH_VARIABLE_VALUE_TYPE_EXISTS,
};

enum CwshVariableValueModifier {
  CWSH_VARIABLE_VALUE_MODIFIER_NONE,
  CWSH_VARIABLE_VALUE_MODIFIER_ROOT,
  CWSH_VARIABLE_VALUE_MODIFIER_EXTENSION,
  CWSH_VARIABLE_VALUE_MODIFIER_HEADER,
  CWSH_VARIABLE_VALUE_MODIFIER_TAIL,
  CWSH_VARIABLE_VALUE_MODIFIER_QUOTE_WORDLIST,
  CWSH_VARIABLE_VALUE_MODIFIER_QUOTE_WORD,
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
