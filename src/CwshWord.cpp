#include <CwshI.h>

namespace Cwsh {

void
Word::
toWords(const std::string &line, WordArray &words)
{
  std::vector<std::string> words1;

  String::addWords(line, words1);

  auto numWords1 = words1.size();

  for (uint i = 0; i < numWords1; i++) {
    const auto &word = words1[i];

    words.push_back(Word(word));
  }
}

std::string
Word::
toString(const WordArray &words)
{
  std::string str;

  auto numWords = words.size();

  for (uint i = 0; i < numWords; i++) {
    if (i > 0)
      str += " ";

    str += words[i].getWord();
  }

  return str;
}

std::string
Word::
toString(const SubWordArray &subWords)
{
  std::string str;

  auto numSubWords = subWords.size();

  for (uint i = 0; i < numSubWords; i++)
    str += subWords[i].getString();

  return str;
}

void
Word::
printWords(const WordArray &words)
{
  auto numWords = words.size();

  for (uint i = 0; i < numWords; i++) {
    if (i > 0)
      std::cerr << " ";

    std::cerr << "'" << words[i] << "'";
  }

  std::cerr << "\n";
}

void
Word::
printWord(const Word &word)
{
  std::cerr << word << "\n";
}

Word::
Word(const std::string &word) :
 word_(word)
{
}

const SubWordArray &
Word::
getSubWords() const
{
  if (! subWordsCreated_) {
    auto *th = const_cast<Word *>(this);

    th->createSubWords();
  }

  return subWords_;
}

void
Word::
createSubWords()
{
  auto len = word_.size();

  std::string sub_word;

  uint i = 0;

  while (i < len) {
    uint i1 = i;

    while (i < len && word_[i] != '\"' && word_[i] != '\'' && word_[i] != '`')
      i++;

    if (i > i1) {
      sub_word = word_.substr(i1, i - i1);

      subWords_.push_back(SubWord(sub_word));
    }

    if (i >= len)
      break;

    if (word_[i] == '\"') {
      uint i2 = i + 1;

      if (! CStrUtil::skipDoubleQuotedString(word_, &i))
        CWSH_THROW("Unmatched \".");

      sub_word = word_.substr(i2, i - i2 - 1);

      subWords_.push_back(SubWord(sub_word, SubWordType::DOUBLE_QUOTED));
    }
    else if (word_[i] == '\'') {
      uint i2 = i + 1;

      if (! CStrUtil::skipSingleQuotedString(word_, &i))
        CWSH_THROW("Unmatched \'.");

      sub_word = word_.substr(i2, i - i2 - 1);

      subWords_.push_back(SubWord(sub_word, SubWordType::SINGLE_QUOTED));
    }
    else if (word_[i] == '`') {
      uint i2 = i + 1;

      if (! CStrUtil::skipBackQuotedString(word_, &i))
        CWSH_THROW("Unmatched `.");

      sub_word = word_.substr(i2, i - i2 - 1);

      subWords_.push_back(SubWord(sub_word, SubWordType::BACK_QUOTED));
    }
  }

  subWordsCreated_ = true;
}

void
Word::
removeQuotes()
{
  const auto &subWords = getSubWords();

  std::string str;

  auto numSubWords = subWords.size();

  for (uint i = 0; i < numSubWords; i++)
    str += subWords[i].getWord();

  word_ = str;

  subWords_.clear();

  subWordsCreated_ = false;
}

std::ostream &
operator<<(std::ostream &os, const Word &word)
{
  os << ">>" << word.word_ << "<<";

  return os;
}

SubWord::
SubWord(const std::string &word, SubWordType type) :
 word_(word), type_(type)
{
}

std::string
SubWord::
getString() const
{
  if      (type_ == SubWordType::SINGLE_QUOTED)
    return "'" + word_ + "'";
  else if (type_ == SubWordType::DOUBLE_QUOTED)
    return "\"" + word_ + "\"";
  else if (type_ == SubWordType::BACK_QUOTED)
    return "`" + word_ + "`";
  else
    return word_;
}

std::ostream &
operator<<(std::ostream &os, const SubWord &sub_word)
{
  os << ">>" << sub_word.getString() << "<<";

  return os;
}

}
