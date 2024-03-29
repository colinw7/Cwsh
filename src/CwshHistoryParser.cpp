#include <CwshI.h>
#include <CwshHistoryParser.h>

namespace Cwsh {

enum class HistoryCommandType {
  NONE,
  QUICK_SUBSTR,
  USE_RESULT,
  SEARCH_START,
  SEARCH_IN,
  SEARCH_ARG
};

enum class HistoryModifierType {
  NONE,
  PRINT,
  SUBSTITUTE,
  REPEAT,
  QUOTE_WORDLIST,
  QUOTE_WORDS,
  ROOT,
  EXTENSION,
  HEADER,
  TAIL
};

class HistoryModifier {
 public:
  HistoryModifier(HistoryModifierType type, bool global,
                  const std::string &old_str, const std::string &new_str) :
   type_(type), global_(global), old_str_(old_str), new_str_(new_str) {
  }

  HistoryModifierType getType     () const { return type_   ; }
  bool                isGlobal    () const { return global_ ; }
  std::string         getOldString() const { return old_str_; }
  std::string         getNewString() const { return new_str_; }

  void print() const;

 private:
  HistoryModifierType type_;
  bool                global_;
  std::string         old_str_;
  std::string         new_str_;
};

using HistoryModifierList = std::vector<HistoryModifier>;

//---

class HistoryOperation {
  CINST_COUNT_MEMBER(HistoryOperation);

 public:
  HistoryOperation(App *cwsh);
 ~HistoryOperation();

  void setStartPos(int pos) { start_pos_ = pos; }
  void setEndPos  (int pos) { end_pos_   = pos; }

  void setCommandType(HistoryCommandType type) { command_type_ = type; }
  void setCommandNum (int                num ) { command_num_  = num ; }

  void setNewString(const std::string &str) { new_str_ = str ; }
  void setOldString(const std::string &str) { old_str_ = str ; }

  void setStartArgNum(int arg_num) { start_arg_num_ = arg_num; }
  void setEndArgNum  (int arg_num) { end_arg_num_   = arg_num; }

  void setForceArgs(bool flag) { force_args_ = flag; }

  void addModifier(const HistoryModifier &modifier) {
    modifiers_.push_back(modifier);
  }

  std::string apply(HistoryParser &parser, const std::string &line);
  std::string apply(HistoryParser &parser, const std::string &line,
                    const std::vector<std::string> &words);

  void display(const std::string &str) const;

 private:
  CPtr<App> cwsh_;

  int start_pos_ { 0 };
  int end_pos_   { 0 };

  HistoryCommandType command_type_ { HistoryCommandType::NONE };
  int                command_num_ { 0 };
  std::string        old_str_;
  std::string        new_str_;

  int  start_arg_num_ { 0 };
  int  end_arg_num_   { -1 };
  bool force_args_    { false };

  HistoryModifierList modifiers_;
};

//---

HistoryParser::
HistoryParser(App *cwsh) :
 cwsh_(cwsh)
{
}

HistoryParser::
~HistoryParser()
{
  for (auto &operation : operations_)
    delete operation;
}

std::string
HistoryParser::
parseLine(const std::string &line)
{
  parse(line);

  if (cwsh_->getDebug())
    display();

  std::string line1 = apply();

  return line1;
}

void
HistoryParser::
parse(const std::string &str)
{
  str_ = str;

  uint len = uint(str_.size());

  if (len > 0 && str_[0] == '^') {
    operation_ = new HistoryOperation(cwsh_);

    operation_->setStartPos(0);

    operations_.push_back(operation_);

    parseQuickSubStr();

    operation_->setEndPos(pos_);
  }
  else {
    while (pos_ < len) {
      if      (str_[pos_] == '\\') {
        pos_++;

        if (pos_ < len)
          pos_++;
      }
      else if (str_[pos_] == '!') {
        if (isCommand()) {
          operation_ = new HistoryOperation(cwsh_);

          operation_->setStartPos(pos_);

          operations_.push_back(operation_);

          parseCommand();

          operation_->setEndPos(pos_);
        }
        else
          pos_++;
      }
      else
        pos_++;
    }
  }
}

bool
HistoryParser::
isCommand()
{
  int pos1 = pos_;

  int len = int(str_.size());

  if (pos1 >= len || str_[pos1] != '!')
    return false;

  pos1++;

  if (pos1 >= len)
    return false;

  if (str_[pos1] == '=' || str_[pos1] == '~' || isspace(str_[pos1]))
    return false;

  return true;
}

void
HistoryParser::
parseCommand()
{
  pos_++;

  //------

  parseSubStr();

  //------

  parseArgSelector();

  //------

  while (parseModifier())
    ;
}

void
HistoryParser::
parseSubStr()
{
  uint len = uint(str_.size());

  if (pos_ < len && str_[pos_] == '!') {
    pos_++;

    int command_num = cwsh_->getHistoryCommandNum();

    operation_->setCommandNum(command_num - 1);

    return;
  }

  if (pos_ < len && str_[pos_] == '#') {
    int pos1 = pos_ - 1;

    pos_++;

    operation_->setCommandType(HistoryCommandType::USE_RESULT);
    operation_->setNewString  (str_.substr(0, pos1));

    return;
  }

  if (pos_ < len && isdigit(str_[pos_])) {
    int pos1 = pos_;

    bool numeric = true;

    while (pos_ < len && isSubStrChar(str_[pos_])) {
      if (! isdigit(str_[pos_]))
        numeric = false;

      pos_++;
    }

    std::string str = str_.substr(pos1, pos_ - pos1);

    if (numeric)
      operation_->setCommandNum(int(CStrUtil::toInteger(str)));
    else {
      operation_->setCommandType(HistoryCommandType::SEARCH_START);
      operation_->setNewString  (str);
    }

    return;
  }

  if (pos_ < len - 1 &&
      str_[pos_] == '-' && isdigit(str_[pos_ + 1])) {
    pos_++;

    int pos1 = pos_;

    bool numeric = true;

    while (pos_ < len && isSubStrChar(str_[pos_])) {
      if (! isdigit(str_[pos_]))
        numeric = false;

      pos_++;
    }

    std::string str = str_.substr(pos1, pos_ - pos1);

    if (numeric) {
      int command_num = cwsh_->getHistoryCommandNum();

      operation_->setCommandNum(command_num - int(CStrUtil::toInteger(str)));
    }
    else {
      operation_->setCommandType(HistoryCommandType::SEARCH_START);
      operation_->setNewString  (str);
    }

    return;
  }

  if (pos_ < len && str_[pos_] == '?') {
    pos_++;

    int pos1 = pos_;

    while (pos_ < len && str_[pos_] != '?')
      pos_++;

    std::string str = str_.substr(pos1, pos_ - pos1);

    if (str_[pos_] == '?')
      pos_++;

    if (pos_ < len && str_[pos_] == '%') {
      pos_++;

      int command_num = cwsh_->getHistoryCommandNum();

      operation_->setCommandType(HistoryCommandType::SEARCH_ARG);
      operation_->setCommandNum (command_num - 1);
      operation_->setNewString  (str);
    }
    else {
      operation_->setCommandType(HistoryCommandType::SEARCH_IN);
      operation_->setNewString  (str);
    }

    return;
  }

  if (pos_ < len && str_[pos_] == '{') {
    pos_++;

    int pos1 = pos_;

    while (pos_ < len && str_[pos_] != '}')
      pos_++;

    std::string str = str_.substr(pos1, pos_ - pos1);

    if (str_[pos_] == '}')
      pos_++;

    operation_->setCommandType(HistoryCommandType::SEARCH_START);
    operation_->setNewString  (str);

    return;
  }

  if (str_[pos_] == '^' || str_[pos_] == '$' ||
      str_[pos_] == '*' || str_[pos_] == ':') {
    int command_num = cwsh_->getHistoryCommandNum();

    operation_->setCommandNum(command_num - 1);

    return;
  }

  int pos1 = pos_;

  while (pos_ < len && isSubStrChar(str_[pos_]))
    pos_++;

  std::string str = str_.substr(pos1, pos_ - pos1);

  operation_->setCommandType(HistoryCommandType::SEARCH_START);
  operation_->setNewString  (str);
}

void
HistoryParser::
parseArgSelector()
{
  bool colon_found = false;

  // Check if we have a Arg Selector

  uint len = uint(str_.size());

  uint pos1 = pos_;

  if (pos1 >= len)
    return;

  if (str_[pos1] == ':') {
    colon_found = true;

    pos1++;
  }

  if (pos1 >= len)
    return;

  if (! (colon_found && isdigit(str_[pos1])) &&
      str_[pos1] != '^' && str_[pos1] != '$' &&
      str_[pos1] != '-' && str_[pos1] != '*')
    return;

  //------

  pos1 = pos_;

  if (str_[pos_] == ':')
    pos_++;

  if      (str_[pos_] == '^') {
    pos_++;

    operation_->setStartArgNum(1);

    if      (pos_ < len && str_[pos_] == '-') {
      pos_++;

      if (pos_ < len && isdigit(str_[pos_])) {
        int integer;

        CStrUtil::readInteger(str_, &pos_, &integer);

        operation_->setEndArgNum(integer);
      }
      else
        pos_--;
    }
    else if (pos_ < len && str_[pos_] == '$') {
      pos_++;

      operation_->setEndArgNum(-1);
    }
    else
      operation_->setEndArgNum(1);
  }
  else if (str_[pos_] == '$') {
    pos_++;

    operation_->setStartArgNum(-1);
    operation_->setEndArgNum  (-1);
  }
  else if (str_[pos_] == '*') {
    pos_++;

    operation_->setStartArgNum(1);
    operation_->setEndArgNum  (-1);
    operation_->setForceArgs  (true);
  }
  else if (str_[pos_] == '-') {
    operation_->setStartArgNum(0);

    pos_++;

    if      (pos_ < len && isdigit(str_[pos_])) {
      int integer;

      CStrUtil::readInteger(str_, &pos_, &integer);

      operation_->setEndArgNum(integer);
    }
    else if (pos_ < len && str_[pos_] == '$') {
      pos_++;

      operation_->setEndArgNum(-1);
    }
    else
      pos_--;
  }
  else if (isdigit(str_[pos_])) {
    int integer;

    CStrUtil::readInteger(str_, &pos_, &integer);

    operation_->setStartArgNum(integer);
    operation_->setEndArgNum  (integer);

    if      (pos_ < len && str_[pos_] == '-') {
      pos_++;

      if      (pos_ < len && isdigit(str_[pos_])) {
        int integer1;

        CStrUtil::readInteger(str_, &pos_, &integer1);

        operation_->setEndArgNum(integer1);
      }
      else if (pos_ < len && str_[pos_] == '$') {
        pos_++;

        operation_->setEndArgNum(-1);
      }
      else
        operation_->setEndArgNum(-2);
    }
    else if (pos_ < len && str_[pos_] == '*')
      operation_->setEndArgNum(-1);
  }
}

bool
HistoryParser::
parseModifier()
{
  // Check if we have a History Modifier

  uint len = uint(str_.size());

  uint pos1 = pos_;

  if (pos1 >= len)
    return false;

  if (str_[pos1] != ':')
    return false;

  pos1++;

  if (pos1 >= len)
    return false;

  if (str_[pos1] == 'g') {
    pos1++;

    if (pos1 >= len)
      return false;

    if (str_[pos1] != 's' && str_[pos1] != 'r' && str_[pos1] != 'e' &&
        str_[pos1] != 'h' && str_[pos1] != 't')
      return false;
  }
  else {
    if (str_[pos1] != 'p' && str_[pos1] != 's' && str_[pos1] != '&' &&
        str_[pos1] != 'q' && str_[pos1] != 'x' && str_[pos1] != 'r' &&
        str_[pos1] != 'e' && str_[pos1] != 'h' && str_[pos1] != 't')
      return false;
  }

  //------

  // Parse History Modifier

  bool global = false;

  pos_++;

  if (str_[pos_] == 'g') {
    pos_++;

    global = true;
  }

  auto type = HistoryModifierType::NONE;

  std::string old_str;
  std::string new_str;

  if      (str_[pos_] == 'p') {
    pos_++;

    type = HistoryModifierType::PRINT;
  }
  else if (str_[pos_] == 's') {
    pos_++;

    type = HistoryModifierType::SUBSTITUTE;

    if (pos_ >= len || str_[pos_] != '/')
      CWSH_THROW("Bad substitute.");

    pos_++;

    int pos2 = pos_;

    while (pos_ < len && str_[pos_] != '/')
      pos_++;

    old_str = str_.substr(pos2, pos_ - pos2);

    if (pos_ < len)
      pos_++;

    pos2 = pos_;

    while (pos_ < len && str_[pos_] != '/')
      pos_++;

    new_str = str_.substr(pos2, pos_ - pos2);

    if (pos_ < len)
      pos_++;
  }
  else if (str_[pos_] == '&') {
    pos_++;

    type = HistoryModifierType::REPEAT;
  }
  else if (str_[pos_] == 'q') {
    pos_++;

    type = HistoryModifierType::QUOTE_WORDLIST;
  }
  else if (str_[pos_] == 'x') {
    pos_++;

    type = HistoryModifierType::QUOTE_WORDS;
  }
  else if (str_[pos_] == 'r') {
    pos_++;

    type = HistoryModifierType::ROOT;
  }
  else if (str_[pos_] == 'e') {
    pos_++;

    type = HistoryModifierType::EXTENSION;
  }
  else if (str_[pos_] == 'h') {
    pos_++;

    type = HistoryModifierType::HEADER;
  }
  else if (str_[pos_] == 't') {
    pos_++;

    type = HistoryModifierType::TAIL;
  }
  else
    CWSH_THROW("Bad modifier.");

  HistoryModifier modifier(type, global, old_str, new_str);

  operation_->addModifier(modifier);

  return true;
}

void
HistoryParser::
parseQuickSubStr()
{
  int command_num = cwsh_->getHistoryCommandNum();

  operation_->setCommandNum (command_num - 1);
  operation_->setCommandType(HistoryCommandType::QUICK_SUBSTR);

  //------

  uint len = uint(str_.size());

  pos_++;

  //------

  uint pos1 = pos_;

  while (pos_ < len && str_[pos_] != '^')
    pos_++;

  operation_->setOldString(str_.substr(pos1, pos_ - pos1));

  if (pos_ < len)
    pos_++;

  //------

  pos1 = pos_;

  while (pos_ < len && str_[pos_] != '^')
    pos_++;

  operation_->setNewString(str_.substr(pos1, pos_ - pos1));

  if (pos_ < len)
    pos_++;
}

std::string
HistoryParser::
apply()
{
  std::string str = str_;

  uint num_operations = uint(operations_.size());

  for (int i = num_operations - 1; i >= 0; i--)
    str = operations_[i]->apply(*this, str);

  if (print_) {
    std::cout << str << "\n";

    throw HistoryIgnore();
  }

  return str;
}

std::string
HistoryParser::
apply(const std::vector<std::string> &words)
{
  std::string str = str_;

  uint num_operations = uint(operations_.size());

  if (num_operations > 0) {
    for (int i = num_operations - 1; i >= 0; i--)
      str = operations_[i]->apply(*this, str, words);
  }
  else
    str += " " + CStrUtil::toString(words, 1);

  if (print_) {
    std::cout << str << "\n";

    throw HistoryIgnore();
  }

  return str;
}

void
HistoryParser::
display() const
{
  uint num_operations = uint(operations_.size());

  for (uint i = 0; i < num_operations; i++)
    operations_[i]->display(str_);
}

bool
HistoryParser::
isSubStrChar(char c)
{
  if (c != '^' && c != '$' && c != '-' && c != '*' &&
      c != '#' && c != ':' && ! isspace(c))
    return true;

  return false;
}

//---------------

HistoryOperation::
HistoryOperation(App *cwsh) :
 cwsh_(cwsh)
{
}

HistoryOperation::
~HistoryOperation()
{
}

std::string
HistoryOperation::
apply(HistoryParser &parser, const std::string &line)
{
  std::string lstr = line.substr(0, start_pos_);

  std::string rstr;

  uint len = uint(line.size());

  if (end_pos_ < int(len))
    rstr = line.substr(end_pos_);

  std::string command;

  if      (command_type_ == HistoryCommandType::QUICK_SUBSTR) {
    command = cwsh_->getHistoryCommand(command_num_);

    std::string::size_type pos = command.find(old_str_);

    if (pos == std::string::npos)
      CWSH_THROW("Modifier failed.");

    std::string::size_type pos1 = pos + old_str_.size() - 1;

    std::string commandl = command.substr(0, pos);

    std::string commandr;

    if (pos1 + 1 < command.size())
      commandr = command.substr(pos1 + 1);

    command = commandl + new_str_ + commandr;
  }
  else if (command_type_ == HistoryCommandType::USE_RESULT)
    command = new_str_;
  else if      (command_type_ == HistoryCommandType::SEARCH_START) {
    if (! cwsh_->findHistoryCommandStart(new_str_, command_num_))
      CWSH_THROW(new_str_ + ": Event not found.");

    command = cwsh_->getHistoryCommand(command_num_);
  }
  else if (command_type_ == HistoryCommandType::SEARCH_IN) {
    if (! cwsh_->findHistoryCommandIn(new_str_, command_num_))
      CWSH_THROW(new_str_ + ": Event not found.");

    command = cwsh_->getHistoryCommand(command_num_);
  }
  else if (command_type_ == HistoryCommandType::SEARCH_ARG) {
    int arg_num;

    if (! cwsh_->findHistoryCommandArg(new_str_, command_num_, arg_num))
      CWSH_THROW(new_str_ + ": Event not found.");

    command = cwsh_->getHistoryCommandArg(command_num_, arg_num);
  }
  else
    command = cwsh_->getHistoryCommand(command_num_);

  CStrWords words = CStrUtil::toWords(command, nullptr);

  if (start_arg_num_ < 0)
    start_arg_num_ += int(words.size());

  if (end_arg_num_ < 0)
    end_arg_num_ += int(words.size());

  if (start_arg_num_ < 0 || start_arg_num_ >= int(words.size()) ||
      end_arg_num_   < 0 || end_arg_num_   >= int(words.size()) ||
      start_arg_num_ > end_arg_num_) {
    if (! force_args_)
      CWSH_THROW("Bad ! arg selector.");

    command = "";
  }
  else {
    if (start_arg_num_ != 0 || end_arg_num_ != int(words.size()) - 1)
      words.truncate(start_arg_num_, end_arg_num_);
  }

  command = CStrUtil::toString(words, " ");

  int num_modifiers = int(modifiers_.size());

  for (int i = 0; i < num_modifiers; i++) {
    HistoryModifierType  type    = modifiers_[i].getType();
    bool                 global  = modifiers_[i].isGlobal();
    const std::string   &new_str = modifiers_[i].getNewString();
    const std::string   &old_str = modifiers_[i].getOldString();

    if      (type == HistoryModifierType::PRINT)
      parser.setPrint(true);
    else if (type == HistoryModifierType::SUBSTITUTE) {
      if (! global) {
        std::string::size_type pos = command.find(old_str);

        if (pos == std::string::npos)
          CWSH_THROW("Modifier failed.");

        std::string::size_type pos1 = pos + old_str.size() - 1;

        std::string commandl = command.substr(0, pos);

        std::string commandr;

        if (pos1 + 1 < command.size())
          commandr = command.substr(pos1 + 1);

        command = commandl + new_str + commandr;
      }
      else {
        std::string::size_type pos = command.rfind(old_str);

        if (pos == std::string::npos)
          CWSH_THROW("Modifier failed.");

        std::string::size_type pos1 = pos + old_str.size() - 1;

        std::string commandl = command.substr(0, pos);

        std::string commandr;

        if (pos1 + 1 < command.size())
          commandr = command.substr(pos1 + 1);

        command = commandl + new_str + commandr;

        std::string::size_type last_pos = pos;

        pos = command.rfind(old_str);

        while (pos != std::string::npos && pos < last_pos) {
          std::string::size_type pos2 = pos + old_str.size() - 1;

          std::string commandl1 = command.substr(0, pos);

          std::string commandr1;

          if (pos2 + 1 < command.size())
            commandr1 = command.substr(pos2 + 1);

          command = commandl1 + new_str + commandr1;

          last_pos = pos;

          pos = command.rfind(old_str);
        }
      }
    }
    else if (type == HistoryModifierType::REPEAT) {
    }
    else if (type == HistoryModifierType::QUOTE_WORDLIST) {
    }
    else if (type == HistoryModifierType::QUOTE_WORDS) {
    }
    else if (type == HistoryModifierType::ROOT) {
      CStrWords rwords = CStrUtil::toWords(command, nullptr);

      int num_words = int(rwords.size());

      for (int ir = 0; ir < num_words; ir++) {
        std::string word = rwords[ir].getWord();

        std::string::size_type pos = word.rfind('.');

        if (pos != std::string::npos)
          rwords.truncateWord(ir, 0, int(pos - 1));

        if (! global)
          break;
      }

      command = CStrUtil::toString(rwords, " ");
    }
    else if (type == HistoryModifierType::EXTENSION) {
      CStrWords ewords = CStrUtil::toWords(command, nullptr);

      int num_words = int(ewords.size());

      for (int ie = 0; ie < num_words; ie++) {
        std::string word = ewords[ie].getWord();

        std::string::size_type pos = word.rfind('.');

        if (pos != std::string::npos)
          ewords.truncateWord(ie, int(pos + 1), int(ewords.size() - 1));

        if (! global)
          break;
      }

      command = CStrUtil::toString(ewords, " ");
    }
    else if (type == HistoryModifierType::HEADER) {
      CStrWords hwords = CStrUtil::toWords(command, nullptr);

      int num_words = int(hwords.size());

      for (int ih = 0; ih < num_words; ih++) {
        std::string word = hwords[ih].getWord();

        std::string::size_type pos = word.rfind('/');

        if (pos != std::string::npos)
          hwords.truncateWord(ih, 0, int(pos - 1));

        if (! global)
          break;
      }

      command = CStrUtil::toString(hwords, " ");
    }
    else if (type == HistoryModifierType::TAIL) {
      CStrWords twords = CStrUtil::toWords(command, nullptr);

      int num_words = int(twords.size());

      for (int it = 0; it < num_words; it++) {
        std::string word = twords[it].getWord();

        std::string::size_type pos = word.rfind('/');

        if (pos != std::string::npos)
          twords.truncateWord(it, int(pos + 1), int(twords.size() - 1));

        if (! global)
          break;
      }

      command = CStrUtil::toString(twords, " ");
    }
  }

  std::string line1 = lstr + command + rstr;

  return line1;
}

std::string
HistoryOperation::
apply(HistoryParser &parser, const std::string &line, const std::vector<std::string> &words)
{
  int command_num = cwsh_->getHistoryCommandNum();

  if (command_num_ != command_num - 1)
    CWSH_THROW("Bad ! arg selector.");

  std::string lstr = line.substr(0, start_pos_);

  std::string rstr;

  int len = int(line.size());

  if (end_pos_ < len)
    rstr = line.substr(end_pos_);

  int num_words = int(words.size());

  if (start_arg_num_ < 0)
    start_arg_num_ += num_words;

  if (end_arg_num_ < 0)
    end_arg_num_ += num_words;

  std::string command;

  if (start_arg_num_ < 0 || start_arg_num_ >= num_words ||
      end_arg_num_   < 0 || end_arg_num_   >= num_words ||
      start_arg_num_ > end_arg_num_) {
    if (! force_args_)
      CWSH_THROW("Bad ! arg selector.");
  }
  else {
    if (start_arg_num_ != 0 || end_arg_num_ != num_words - 1)
      command = CStrUtil::toString(words, start_arg_num_, end_arg_num_);
    else
      command = CStrUtil::toString(words, " ");
  }

  int num_modifiers = int(modifiers_.size());

  for (int i = 0; i < num_modifiers; i++) {
    HistoryModifierType  type    = modifiers_[i].getType();
    bool                 global  = modifiers_[i].isGlobal();
    const std::string   &new_str = modifiers_[i].getNewString();
    const std::string   &old_str = modifiers_[i].getOldString();

    if      (type == HistoryModifierType::PRINT)
      parser.setPrint(true);
    else if (type == HistoryModifierType::SUBSTITUTE) {
      if (! global) {
        std::string::size_type pos = command.find(old_str);

        if (pos == std::string::npos)
          CWSH_THROW("Modifier failed.");

        std::string::size_type pos1 = pos + old_str.size() - 1;

        std::string commandl = command.substr(0, pos);

        std::string commandr;

        if (pos1 + 1 < command.size())
          commandr = command.substr(pos1 + 1);

        command = commandl + new_str + commandr;
      }
      else {
        std::string::size_type pos = command.rfind(old_str);

        if (pos == std::string::npos)
          CWSH_THROW("Modifier failed.");

        std::string::size_type pos1 = pos + old_str.size() - 1;

        std::string commandl = command.substr(0, pos);

        std::string commandr;

        if (pos1 + 1 < command.size())
          commandr = command.substr(pos1 + 1);

        command = commandl + new_str + commandr;

        std::string::size_type last_pos = pos;

        pos = command.rfind(old_str);

        while (pos != std::string::npos && pos < last_pos) {
          std::string::size_type pos2 = pos + old_str.size() - 1;

          std::string commandl1 = command.substr(0, pos);

          std::string commandr1;

          if (pos2 + 1 < command.size())
            commandr1 = command.substr(pos2 + 1);

          command = commandl1 + new_str + commandr1;

          last_pos = pos;

          pos = command.rfind(old_str);
        }
      }
    }
    else if (type == HistoryModifierType::REPEAT) {
    }
    else if (type == HistoryModifierType::QUOTE_WORDLIST) {
    }
    else if (type == HistoryModifierType::QUOTE_WORDS) {
    }
    else if (type == HistoryModifierType::ROOT) {
      CStrWords words1 = CStrUtil::toWords(command, nullptr);

      int num_words1 = int(words1.size());

      for (int ir = 0; ir < num_words1; ir++) {
        std::string word = words1[ir].getWord();

        std::string::size_type pos = word.rfind('.');

        if (pos != std::string::npos)
          words1.truncateWord(ir, 0, int(pos - 1));

        if (! global)
          break;
      }

      command = CStrUtil::toString(words1, " ");
    }
    else if (type == HistoryModifierType::EXTENSION) {
      CStrWords words1 = CStrUtil::toWords(command, nullptr);

      int num_words1 = int(words1.size());

      for (int ie = 0; ie < num_words1; ie++) {
        std::string word = words1[ie].getWord();

        std::string::size_type pos = word.rfind('.');

        if (pos != std::string::npos)
          words1.truncateWord(ie, int(pos + 1), int(words1.size() - 1));

        if (! global)
          break;
      }

      command = CStrUtil::toString(words1, " ");
    }
    else if (type == HistoryModifierType::HEADER) {
      CStrWords words1 = CStrUtil::toWords(command, nullptr);

      int num_words1 = int(words1.size());

      for (int ih = 0; ih < num_words1; ih++) {
        std::string word = words1[ih].getWord();

        std::string::size_type pos = word.rfind('/');

        if (pos != std::string::npos)
          words1.truncateWord(ih, 0, int(pos - 1));

        if (! global)
          break;
      }

      command = CStrUtil::toString(words1, " ");
    }
    else if (type == HistoryModifierType::TAIL) {
      CStrWords words1 = CStrUtil::toWords(command, nullptr);

      int num_words1 = int(words1.size());

      for (int it = 0; it < num_words1; it++) {
        std::string word = words1[it].getWord();

        std::string::size_type pos = word.rfind('/');

        if (pos != std::string::npos)
          words1.truncateWord(it, int(pos + 1), int(words1.size() - 1));

        if (! global)
          break;
      }

      command = CStrUtil::toString(words1, " ");
    }
  }

  std::string line1 = lstr + command + rstr;

  return line1;
}

void
HistoryOperation::
display(const std::string &str) const
{
  std::string command = str.substr(start_pos_, end_pos_ - start_pos_);

  std::cout << "Command " << command << "\n";

  if      (command_type_ == HistoryCommandType::QUICK_SUBSTR)
    std::cout << "Replace " << old_str_ << " with " << new_str_ << "\n";
  else if (command_type_ == HistoryCommandType::USE_RESULT)
    std::cout << "Result " << new_str_ << "\n";
  else {
    if      (command_type_ == HistoryCommandType::SEARCH_START)
      std::cout << "Last Command with " << new_str_ << " at start\n";
    else if (command_type_ == HistoryCommandType::SEARCH_IN)
      std::cout << "Last Command with " << new_str_ << " anywhere\n";
    else if (command_type_ == HistoryCommandType::SEARCH_ARG)
      std::cout << "Last Arg with " << new_str_ << " anywhere\n";
    else
      std::cout << "Command Num " << command_num_ << "\n";

    std::cout << "Args " << start_arg_num_ << " " << end_arg_num_ << "\n";
  }

  int num_modifiers = int(modifiers_.size());

  for (int i = 0; i < num_modifiers; i++)
    modifiers_[i].print();
}

void
HistoryModifier::
print() const
{
  if      (type_ == HistoryModifierType::PRINT)
    std::cout << "Print\n";
  else if (type_ == HistoryModifierType::SUBSTITUTE) {
    if (global_)
      std::cout << "Globally ";

    std::cout << "Substitute " << old_str_ << " with " << new_str_ << "\n";
  }
  else if (type_ == HistoryModifierType::REPEAT) {
    if (global_)
      std::cout << "Globally ";

    std::cout << "Repeat\n";
  }
  else if (type_ == HistoryModifierType::QUOTE_WORDLIST)
    std::cout << "Quote Wordlist";
  else if (type_ == HistoryModifierType::QUOTE_WORDS)
    std::cout << "Quote Words";
  else if (type_ == HistoryModifierType::ROOT) {
    if (global_)
      std::cout << "Globally ";

    std::cout << "Root\n";
  }
  else if (type_ == HistoryModifierType::EXTENSION) {
    if (global_)
      std::cout << "Globally ";

    std::cout << "Extension\n";
  }
  else if (type_ == HistoryModifierType::HEADER) {
    if (global_)
      std::cout << "Globally ";

    std::cout << "Header\n";
  }
  else if (type_ == HistoryModifierType::TAIL) {
    if (global_)
      std::cout << "Globally ";

    std::cout << "Tail\n";
  }
}

}
