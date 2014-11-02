#include <CwshI.h>

void
CwshWord::
toWords(const string &line, CwshWordArray &words)
{
  vector<string> words1;

  CwshString::addWords(line, words1);

  int num_words1 = words1.size();

  for (int i = 0; i < num_words1; i++) {
    const string &word = words1[i];

    words.push_back(CwshWord(word));
  }
}

string
CwshWord::
toString(const CwshWordArray &words)
{
  string str;

  int num_words = words.size();

  for (int i = 0; i < num_words; i++) {
    if (i > 0)
      str += " ";

    str += words[i].getWord();
  }

  return str;
}

string
CwshWord::
toString(const CwshSubWordArray &sub_words)
{
  string str;

  int num_sub_words = sub_words.size();

  for (int i = 0; i < num_sub_words; i++)
    str += sub_words[i].getString();

  return str;
}

void
CwshWord::
printWords(const CwshWordArray &words)
{
  int num_words = words.size();

  for (int i = 0; i < num_words; i++) {
    if (i > 0)
      std::cerr << " ";

    std::cerr << "'" << words[i] << "'";
  }

  std::cerr << std::endl;
}

void
CwshWord::
printWord(const CwshWord &word)
{
  std::cerr << word << std::endl;
}

CwshWord::
CwshWord(const string &word) :
 word_(word)
{
  sub_words_created_ = false;
}

const CwshSubWordArray &
CwshWord::
getSubWords() const
{
  if (! sub_words_created_) {
    CwshWord *th = const_cast<CwshWord *>(this);

    th->createSubWords();
  }

  return sub_words_;
}

void
CwshWord::
createSubWords()
{
  uint len = word_.size();

  string sub_word;

  uint i = 0;

  while (i < len) {
    uint i1 = i;

    while (i < len && word_[i] != '\"' && word_[i] != '\'' && word_[i] != '`')
      i++;

    if (i > i1) {
      sub_word = word_.substr(i1, i - i1);

      sub_words_.push_back(CwshSubWord(sub_word));
    }

    if (i >= len)
      break;

    if (word_[i] == '\"') {
      uint i1 = i + 1;

      if (! CStrUtil::skipDoubleQuotedString(word_, &i))
        CWSH_THROW("Unmatched \".");

      sub_word = word_.substr(i1, i - i1 - 1);

      sub_words_.push_back(
       CwshSubWord(sub_word, CWSH_SUB_WORD_TYPE_DOUBLE_QUOTED));
    }
    else if (word_[i] == '\'') {
      uint i1 = i + 1;

      if (! CStrUtil::skipSingleQuotedString(word_, &i))
        CWSH_THROW("Unmatched \'.");

      sub_word = word_.substr(i1, i - i1 - 1);

      sub_words_.push_back(
       CwshSubWord(sub_word, CWSH_SUB_WORD_TYPE_SINGLE_QUOTED));
    }
    else if (word_[i] == '`') {
      uint i1 = i + 1;

      if (! CStrUtil::skipBackQuotedString(word_, &i))
        CWSH_THROW("Unmatched `.");

      sub_word = word_.substr(i1, i - i1 - 1);

      sub_words_.push_back(
       CwshSubWord(sub_word, CWSH_SUB_WORD_TYPE_BACK_QUOTED));
    }
  }

  sub_words_created_ = true;
}

void
CwshWord::
removeQuotes()
{
  const CwshSubWordArray &sub_words = getSubWords();

  string str;

  int num_sub_words = sub_words.size();

  for (int i = 0; i < num_sub_words; i++)
    str += sub_words[i].getWord();

  word_ = str;

  sub_words_.clear();

  sub_words_created_ = false;
}

ostream &
operator<<(ostream &os, const CwshWord &word)
{
  os << ">>" << word.word_ << "<<";

  return os;
}

CwshSubWord::
CwshSubWord(const string &word, CwshSubWordType type) :
 word_(word), type_(type)
{
}

string
CwshSubWord::
getString() const
{
  if      (type_ == CWSH_SUB_WORD_TYPE_SINGLE_QUOTED)
    return "'" + word_ + "'";
  else if (type_ == CWSH_SUB_WORD_TYPE_DOUBLE_QUOTED)
    return "\"" + word_ + "\"";
  else if (type_ == CWSH_SUB_WORD_TYPE_BACK_QUOTED)
    return "`" + word_ + "`";
  else
    return word_;
}

ostream &
operator<<(ostream &os, const CwshSubWord &sub_word)
{
  os << ">>" << sub_word.getString() << "<<";

  return os;
}
