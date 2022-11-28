#include <CwshI.h>

namespace Cwsh {

bool
Braces::
expand(const Word &word, WordArray &words)
{
  auto str = word.getWord();
  auto len = uint(str.size());

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

  WordArray words1;

  auto str1 = str.substr(0, i1 - 1);
  auto str2 = str.substr(i2 + 2);

  int j = i1;
  int k = j;

  while (j <= i2) {
    if      (str[j] == '\\') {
      j++;

      if (j < int(len))
        j++;
    }
    else if (str[j] == ',') {
      std::string word1 = str1 + str.substr(k, j - k) + str2;

      words1.push_back(Word(word1));

      j++;

      k = j;
    }
    else
      j++;
  }

  auto word1 = str1 + str.substr(k, j - k) + str2;

  words1.push_back(Word(word1));

  //------

  auto num_words = uint(words1.size());

  for (i = 0; i < num_words; i++) {
    const auto &cword1 = words1[i];

    WordArray words2;

    if (expand(cword1, words2))
      copy(words2.begin(), words2.end(), back_inserter(words));
    else
      words.push_back(cword1);
  }

  //------

  return true;
}

}
