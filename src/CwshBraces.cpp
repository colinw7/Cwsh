#include <CwshI.h>

bool
CwshBraces::
expand(const CwshWord &word, CwshWordArray &words)
{
  std::string str = word.getWord();

  uint len = str.size();

  uint i = 0;

  while (i < len) {
    if      (str[i] == '\"') {
      if (! CStrUtil::skipDoubleQuotedString(str, &i))
        CWSH_THROW("Unmatched \".");
    }
    else if (str[i] == '\'') {
      if (! CStrUtil::skipSingleQuotedString(str, &i))
        CWSH_THROW("Unmatched \'.");
    }
    else if (str[i] == '\\') {
      i++;

      if (i < len)
        i++;
    }
    else if (str[i] == '{')
      break;
    else
      i++;
  }

  if (i >= len)
    return false;

  i++;

  /* Save Start of String inside Braces */

  int i1 = i;

  /* Find Closing Brace */

  while (i < len) {
    if      (str[i] == '\"') {
      if (! CStrUtil::skipDoubleQuotedString(str, &i))
        CWSH_THROW("Unmatched \".");
    }
    else if (str[i] == '\'') {
      if (! CStrUtil::skipSingleQuotedString(str, &i))
        CWSH_THROW("Unmatched \'.");
    }
    else if (str[i] == '\\') {
      i++;

      if (i < len)
        i++;
    }
    else if (str[i] == '}')
      break;
    else
      i++;
  }

  if (i >= len)
    return false;

  int i2 = i - 1;

  //------

  /* Get Comma Separated Strings */

  CwshWordArray words1;

  std::string str1 = str.substr(0, i1 - 1);
  std::string str2 = str.substr(i2 + 2);

  int j = i1;
  int k = j;

  while (j <= i2) {
    if      (str[j] == '\\') {
      j++;

      if (j < (int) len)
        j++;
    }
    else if (str[j] == ',') {
      std::string word1 = str1 + str.substr(k, j - k) + str2;

      words1.push_back(CwshWord(word1));

      j++;

      k = j;
    }
    else
      j++;
  }

  std::string word1 = str1 + str.substr(k, j - k) + str2;

  words1.push_back(CwshWord(word1));

  //------

  uint num_words = words1.size();

  for (i = 0; i < num_words; i++) {
    const CwshWord &word1 = words1[i];

    CwshWordArray words2;

    if (expand(word1, words2))
      copy(words2.begin(), words2.end(), back_inserter(words));
    else
      words.push_back(word1);
  }

  //------

  return true;
}
