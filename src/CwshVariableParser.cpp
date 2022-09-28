#include <CwshI.h>
#include <COSProcess.h>

CwshVariableParser::
CwshVariableParser(Cwsh *cwsh, const CwshWord &word) :
 cwsh_(cwsh), word_(word)
{
}

bool
CwshVariableParser::
expandVariables(CwshWordArray &words)
{
  const CwshSubWordArray &sub_words = word_.getSubWords();

  std::string word1;

  int num_sub_words = int(sub_words.size());

  for (int i = 0; i < num_sub_words; i++) {
    CwshSubWordType type = sub_words[i].getType();

    if      (type == CwshSubWordType::SINGLE_QUOTED) {
      std::string sub_word = sub_words[i].getString();

      word1 += sub_word;
    }
    else if (type == CwshSubWordType::BACK_QUOTED) {
      std::string sub_word = sub_words[i].getString();

      word1 += sub_word;
    }
    else if (type == CwshSubWordType::DOUBLE_QUOTED) {
      std::string sub_word =
        expandQuotedVariables(CwshWord(sub_words[i].getWord()));

      word1 += '"' + sub_word + '"';
    }
    else {
      std::vector<std::string> sub_words1;

      expandVariables1(sub_words[i].getString(), sub_words1);

      sub_words1[0] = word1 + sub_words1[0];

      int num_sub_words1 = int(sub_words1.size());

      for (int j = 0; j < num_sub_words1 - 1; j++)
        words.push_back(sub_words1[j]);

      word1 = sub_words1[num_sub_words1 - 1];
    }
  }

  words.push_back(word1);

  return true;
}

std::string
CwshVariableParser::
expandQuotedVariables(const CwshWord &word)
{
  CwshWordArray words;

  const CwshSubWordArray &sub_words = word.getSubWords();

  std::string word1;

  int num_sub_words = int(sub_words.size());

  for (int i = 0; i < num_sub_words; i++) {
    CwshSubWordType type = sub_words[i].getType();

    if      (type == CwshSubWordType::SINGLE_QUOTED) {
      std::string sub_word = sub_words[i].getString();

      word1 += sub_word;
    }
    else if (type == CwshSubWordType::BACK_QUOTED) {
      std::string sub_word = sub_words[i].getString();

      word1 += sub_word;
    }
    else {
      std::vector<std::string> sub_words1;

      expandVariables1(sub_words[i].getString(), sub_words1);

      int num_sub_words1 = int(sub_words1.size());

      for (int j = 0; j < num_sub_words1; j++)
        word1 += sub_words1[j];
    }
  }

  return word1;
}

bool
CwshVariableParser::
expandVariables1(const std::string &str, std::vector<std::string> &words)
{
  std::string word;

  int i = 0;

  int len = int(str.size());

  while (i < len) {
    int j = i;

    while (i < len) {
      if (str[i] == '$')
        break;

      if (str[i] == '\\' && i < len - 1)
        i++;

      i++;
    }

    word += str.substr(j, i - j);

    if (i >= len - 1)
      break;

    //------

    j = i++;

    std::string name;

    if (i < len && str[i] == '{') {
      i++;

      int k = i;

      while (i < len && str[i] != '}')
        i++;

      if (i >= len)
        CWSH_THROW("Missing }");

      i++;

      name = str.substr(k, i - k - 1);

      if (name == "")
        CWSH_THROW("Illegal variable name.");
    }
    else {
      int k = i;

      if (i < len && (str[i] == '#' || str[i] == '?'))
        i++;

      if      (i < len && (str[i] == '<' || str[i] == '$' || str[i] == '*'))
        i++;
      else if (i < len && isdigit(str[i])) {
        i++;

        while (i < len && isdigit(str[i]))
          i++;
      }
      else if (i < len && (isalpha(str[i]) || str[i] == '_')) {
        i++;

        while (i < len && (isalnum(str[i]) || str[i] == '_'))
          i++;

        if (i < len && str[i] == '[') {
          i++;

          while (i < len && str[i] != ']')
            i++;

          if (i >= len)
            CWSH_THROW("Newline in variable index.");

          i++;
        }
      }

      if (i < len && str[i] == ':') {
        int k1 = i + 1;

        if (k1 < len && str[k1] == 'g') {
          k1++;

          if (k1 < len &&
              (str[k1] == 'r' || str[k1] == 'e' ||
               str[k1] == 'h' || str[k1] == 't'))
            i = k1 + 1;
        }
        else {
          if (k1 < len &&
              (str[k1] == 'r' || str[k1] == 'e' ||
               str[k1] == 'h' || str[k1] == 't' ||
               str[k1] == 'q' || str[k1] == 'z'))
            i = k1 + 1;
        }
      }

      name = str.substr(k, i - k);

      if (name == "")
        continue;
    }

    std::vector<std::string> values;

    expandVariable(name, values);

    //------

    values[0] = word + values[0];

    int num_values = int(values.size());

    for (int k = 0; k < num_values - 1; k++)
      words.push_back(values[k]);

    word = values[num_values - 1];
  }

  words.push_back(word);

  return true;
}

bool
CwshVariableParser::
expandVariable(const std::string &name, std::vector<std::string> &words)
{
  if      (name[0] == '<') {
    std::string word = cwsh_->readLine();

    words.push_back(word);

    return true;
  }

  if (name[0] == '$') {
    int pid = COSProcess::getProcessId();

    std::string word = CStrUtil::toString(pid) + name.substr(1);

    words.push_back(word);

    return true;
  }

  CwshVariableValueType type = CwshVariableValueType::VALUE;

  std::string name1;

  if      (name[0] == '#') {
    type = CwshVariableValueType::SIZE;

    name1 = name.substr(1);

    if (name1.size() == 0) {
      std::string word = "0";

      words.push_back(word);

      return true;
    }
  }
  else if (name[0] == '?') {
    type = CwshVariableValueType::EXISTS;

    name1 = name.substr(1);

    if (name1.size() == 0) {
      std::string word = "1";

      words.push_back(word);

      return true;
    }
  }
  else
    name1 = name;

  //------

  if      (name1[0] == '*') {
    if (type == CwshVariableValueType::SIZE ||
        type == CwshVariableValueType::EXISTS)
      CWSH_THROW("* not allowed with $# or $?.");

    name1 = "argv[*]" + name1.substr(1);
  }

  //------

  if (isdigit(name1[0])) {
    int len1 = int(name1.size());

    int i = 1;

    while (i < len1 && isdigit(name1[i]))
      i++;

    std::string num_str = name1.substr(0, i);

    int num = int(CStrUtil::toInteger(num_str));

    if (type == CwshVariableValueType::SIZE)
      CWSH_THROW("$#<num> is not allowed.");

    if (type == CwshVariableValueType::EXISTS) {
      if (num > 0)
        CWSH_THROW("$?<num> is not allowed.");

       name1 = "argv" + name1.substr(i);
     }
     else
       name1 = "argv[" + num_str + "]" + name1.substr(i);
  }

  //------

  if (name1[0] != '_' && ! isalpha(name1[0]))
    CWSH_THROW(name1 + ": Invalid variable name.");

  //------

  int len1 = int(name1.size());

  int i = 1;

  while (i < len1 && (name1[i] == '_' || isalnum(name1[i])))
    i++;

  std::string variable_name = name1.substr(0, i);
  std::string subscript_str = name1.substr(i);

  //------

  std::vector<std::string> variable_values;

  CwshVariable *variable = cwsh_->lookupVariable(variable_name);

  if (variable) {
    if (type == CwshVariableValueType::EXISTS) {
      std::string word = "1";

      word += subscript_str;

      words.push_back(word);

      return true;
    }

    variable_values = variable->getValues();
  }
  else {
    if (type == CwshVariableValueType::EXISTS) {
      std::string word = "0";

      if (CEnvInst.exists(variable_name))
        word = "1";

      word += subscript_str;

      words.push_back(word);

      return true;
    }

    if (! CEnvInst.exists(variable_name))
      CWSH_THROW(variable_name + ": Undefined variable.");

    std::vector<std::string> values = CEnvInst.getValues(variable_name);

    int num_values = int(values.size());

    for (int iv = 0; iv < num_values; iv++)
      variable_values.push_back(values[iv]);
  }

  //------

  if (type == CwshVariableValueType::SIZE) {
    std::string word = CStrUtil::toString(int(variable_values.size()));

    word += subscript_str;

    words.push_back(word);

    return true;
  }

  //------

  int subscript_len = int(subscript_str.size());

  int start_value, end_value;

  if (subscript_len > 0 && subscript_str[0] == '[') {
    int is = 1;

    CStrUtil::skipSpace(subscript_str, &is);

    if (is >= subscript_len ||
        (subscript_str[is] != '*' && subscript_str[is] != '-' &&
         ! isdigit(subscript_str[is])))
      CWSH_THROW("Variable Syntax.");

    if      (subscript_str[is] == '*') {
      start_value = 1;
      end_value   = int(variable_values.size());

      is++;
    }
    else {
      if (isdigit(subscript_str[is])) {
        int j = is;

        is++;

        while (is < subscript_len && isdigit(subscript_str[is]))
          is++;

        std::string num_str = subscript_str.substr(j, is - j);

        start_value = int(CStrUtil::toInteger(num_str));

        int num_variable_values = int(variable_values.size());

        if (variable_name == "argv") {
          if (start_value <  0 || start_value > num_variable_values)
            CWSH_THROW(variable_name + ": Subscript out of range.");
        }
        else {
          if (start_value <= 0 || start_value > num_variable_values)
            CWSH_THROW(variable_name + ": Subscript out of range.");
        }

        CStrUtil::skipSpace(subscript_str, &is);
      }
      else
        start_value = -1;

      if (is < subscript_len && subscript_str[is] == '-') {
        is++;

        CStrUtil::skipSpace(subscript_str, &is);

        if (isdigit(subscript_str[is])) {
          if (start_value == -1)
            start_value = 1;

          int j = is;

          is++;

          while (is < subscript_len && isdigit(subscript_str[is]))
            is++;

          std::string num_str = subscript_str.substr(j, is - j);

          end_value = int(CStrUtil::toInteger(num_str));

          int num_variable_values = int(variable_values.size());

          if (end_value <= 0 || end_value > num_variable_values)
            CWSH_THROW(variable_name + ": Subscript out of range.");
        }
        else {
          if (start_value == -1)
            CWSH_THROW("Variable Syntax.");

          end_value = int(variable_values.size());
        }
      }
      else
        end_value = start_value;
    }

    CStrUtil::skipSpace(subscript_str, &is);

    if (is < subscript_len && subscript_str[is] != ']')
      CWSH_THROW("Variable Syntax.");

    is++;

    if (start_value > end_value)
      CWSH_THROW(variable_name + ": Invalid subscript range.");

    subscript_str = subscript_str.substr(is);

    subscript_len = int(subscript_str.size());
  }
  else {
    start_value = 1;
    end_value   = int(variable_values.size());
  }

  CwshVariableValueModifier modifier = CwshVariableValueModifier::NONE;

  bool modifier_global = false;

  if (subscript_len > 0 && subscript_str[0] == ':') {
    int is = 1;

    if (is >= subscript_len)
      CWSH_THROW("Variable Syntax.");

    if (is < subscript_len - 1 &&
        subscript_str[is] == 'g' &&
        (subscript_str[is + 1] == 'r' || subscript_str[is + 1] == 'e' ||
         subscript_str[is + 1] == 'h' || subscript_str[is + 1] == 't')) {
      is++;

      modifier_global = true;
    }

    switch (subscript_str[is]) {
      case 'r':
        modifier = CwshVariableValueModifier::ROOT;
        break;
      case 'e':
        modifier = CwshVariableValueModifier::EXTENSION;
        break;
      case 'h':
        modifier = CwshVariableValueModifier::HEADER;
        break;
      case 't':
        modifier = CwshVariableValueModifier::TAIL;
        break;
      case 'q':
        modifier = CwshVariableValueModifier::QUOTE_WORDLIST;
        break;
      case 'x':
        modifier = CwshVariableValueModifier::QUOTE_WORD;
        break;
      default:
        CWSH_THROW(std::string("Bad : modifier in $ (") + subscript_str[is] + ").");
        break;
    }

    is++;

    subscript_str = subscript_str.substr(is);
  }

  std::vector<std::string> values;

  if (start_value == 0) {
    values.push_back(cwsh_->getArgv0());

    start_value++;
  }

  int num_variable_values = int(variable_values.size());

  for (i = start_value; i <= end_value; i++) {
    if      (i >= 1 && i <= num_variable_values)
      values.push_back(variable_values[i - 1]);
    else if (i == 0)
      values.push_back(cwsh_->getName());
    else
      values.push_back("");
  }

  if      (modifier == CwshVariableValueModifier::ROOT ||
           modifier == CwshVariableValueModifier::EXTENSION ||
           modifier == CwshVariableValueModifier::HEADER ||
           modifier == CwshVariableValueModifier::TAIL) {
    int num_values = int(values.size());

    for (int iv = 0; iv < num_values; iv++) {
      if      (modifier == CwshVariableValueModifier::ROOT) {
        std::string::size_type pos = values[iv].rfind('.');

        if (pos != std::string::npos)
          values[iv] = values[iv].substr(0, pos);
      }
      else if (modifier == CwshVariableValueModifier::EXTENSION) {
        std::string::size_type pos = values[iv].rfind('.');

        if (pos != std::string::npos)
          values[iv] = values[iv].substr(pos + 1);
      }
      else if (modifier == CwshVariableValueModifier::HEADER) {
        std::string::size_type pos = values[iv].rfind('/');

        if (pos != std::string::npos)
          values[iv] = values[iv].substr(0, pos);
      }
      else if (modifier == CwshVariableValueModifier::TAIL) {
        std::string::size_type pos = values[iv].rfind('/');

        if (pos != std::string::npos)
          values[iv] = values[iv].substr(pos + 1);
      }

      if (! modifier_global)
        break;
    }
  }
  else if (modifier == CwshVariableValueModifier::QUOTE_WORDLIST ||
           modifier == CwshVariableValueModifier::QUOTE_WORD) {
    std::string value;

    int num_values = int(values.size());

    for (int iv = 0; iv < num_values; iv++) {
      if (iv > 0)
        value += " ";

      value += values[iv];
    }

    values.clear();

    values.push_back(value);
  }

  if (values.size() == 0)
    values.push_back("");

  values[0] += subscript_str;

  copy(values.begin(), values.end(), back_inserter(words));

  return true;
}
