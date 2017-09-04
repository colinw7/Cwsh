#include <CwshI.h>
#include <CwshHistoryParser.h>

enum class CwshHistoryCommandType {
  NONE,
  QUICK_SUBSTR,
  USE_RESULT,
  SEARCH_START,
  SEARCH_IN,
  SEARCH_ARG
};

enum class CwshHistoryModifierType {
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

class CwshHistoryModifier {
 private:
  CwshHistoryModifierType type_;
  bool                    global_;
  std::string             old_str_;
  std::string             new_str_;

 public:
  CwshHistoryModifier(CwshHistoryModifierType type, bool global,
                      const std::string &old_str, const std::string &new_str) :
   type_(type), global_(global), old_str_(old_str), new_str_(new_str) {
  }

  CwshHistoryModifierType getType     () const { return type_   ; }
  bool                    isGlobal    () const { return global_ ; }
  std::string             getOldString() const { return old_str_; }
  std::string             getNewString() const { return new_str_; }

  void print() const;
};

typedef std::vector<CwshHistoryModifier> CwshHistoryModifierList;

//---

class CwshHistoryOperation {
  CINST_COUNT_MEMBER(CwshHistoryOperation);

 public:
  CwshHistoryOperation(Cwsh *cwsh);
 ~CwshHistoryOperation();

  void setStartPos(int pos) { start_pos_ = pos; }
  void setEndPos  (int pos) { end_pos_   = pos; }

  void setCommandType(CwshHistoryCommandType type) { command_type_ = type; }
  void setCommandNum (int                    num ) { command_num_  = num ; }

  void setNewString(const std::string &str) { new_str_ = str ; }
  void setOldString(const std::string &str) { old_str_ = str ; }

  void setStartArgNum(int arg_num) { start_arg_num_ = arg_num; }
  void setEndArgNum  (int arg_num) { end_arg_num_   = arg_num; }

  void setForceArgs(bool flag) { force_args_ = flag; }

  void addModifier(const CwshHistoryModifier &modifier) {
    modifiers_.push_back(modifier);
  }

  std::string apply(CwshHistoryParser &parser, const std::string &line);
  std::string apply(CwshHistoryParser &parser, const std::string &line,
                    const std::vector<std::string> &words);

  void display(const std::string &str) const;

 private:
  CPtr<Cwsh>              cwsh_;

  int                     start_pos_;
  int                     end_pos_;

  CwshHistoryCommandType  command_type_;
  int                     command_num_;
  std::string             old_str_;
  std::string             new_str_;

  int                     start_arg_num_;
  int                     end_arg_num_;
  bool                    force_args_;

  CwshHistoryModifierList modifiers_;
};

//---

CwshHistoryParser::
CwshHistoryParser(Cwsh *cwsh) :
 cwsh_(cwsh)
{
}

CwshHistoryParser::
~CwshHistoryParser()
{
  for (auto &operation : operations_)
    delete operation;
}

std::string
CwshHistoryParser::
parseLine(const std::string &line)
{
  parse(line);

  if (cwsh_->getDebug())
    display();

  std::string line1 = apply();

  return line1;
}

void
CwshHistoryParser::
parse(const std::string &str)
{
  str_ = str;

  uint len = str_.size();

  if (len > 0 && str_[0] == '^') {
    operation_ = new CwshHistoryOperation(cwsh_);

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
          operation_ = new CwshHistoryOperation(cwsh_);

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
CwshHistoryParser::
isCommand()
{
  int pos1 = pos_;

  int len = str_.size();

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
CwshHistoryParser::
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
CwshHistoryParser::
parseSubStr()
{
  uint len = str_.size();

  if (pos_ < len && str_[pos_] == '!') {
    pos_++;

    int command_num = cwsh_->getHistoryCommandNum();

    operation_->setCommandNum(command_num - 1);

    return;
  }

  if (pos_ < len && str_[pos_] == '#') {
    int pos1 = pos_ - 1;

    pos_++;

    operation_->setCommandType(CwshHistoryCommandType::USE_RESULT);
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
      operation_->setCommandNum(CStrUtil::toInteger(str));
    else {
      operation_->setCommandType(CwshHistoryCommandType::SEARCH_START);
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

      operation_->setCommandNum(command_num - CStrUtil::toInteger(str));
    }
    else {
      operation_->setCommandType(CwshHistoryCommandType::SEARCH_START);
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

      operation_->setCommandType(CwshHistoryCommandType::SEARCH_ARG);
      operation_->setCommandNum (command_num - 1);
      operation_->setNewString  (str);
    }
    else {
      operation_->setCommandType(CwshHistoryCommandType::SEARCH_IN);
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

    operation_->setCommandType(CwshHistoryCommandType::SEARCH_START);
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

  operation_->setCommandType(CwshHistoryCommandType::SEARCH_START);
  operation_->setNewString  (str);
}

void
CwshHistoryParser::
parseArgSelector()
{
  bool colon_found = false;

  // Check if we have a Arg Selector

  uint len = str_.size();

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
        int integer;

        CStrUtil::readInteger(str_, &pos_, &integer);

        operation_->setEndArgNum(integer);
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
CwshHistoryParser::
parseModifier()
{
  // Check if we have a History Modifier

  uint len = str_.size();

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

  CwshHistoryModifierType type = CwshHistoryModifierType::NONE;

  std::string old_str;
  std::string new_str;

  if      (str_[pos_] == 'p') {
    pos_++;

    type = CwshHistoryModifierType::PRINT;
  }
  else if (str_[pos_] == 's') {
    pos_++;

    type = CwshHistoryModifierType::SUBSTITUTE;

    if (pos_ >= len || str_[pos_] != '/')
      CWSH_THROW("Bad substitute.");

    pos_++;

    int pos1 = pos_;

    while (pos_ < len && str_[pos_] != '/')
      pos_++;

    old_str = str_.substr(pos1, pos_ - pos1);

    if (pos_ < len)
      pos_++;

    pos1 = pos_;

    while (pos_ < len && str_[pos_] != '/')
      pos_++;

    new_str = str_.substr(pos1, pos_ - pos1);

    if (pos_ < len)
      pos_++;
  }
  else if (str_[pos_] == '&') {
    pos_++;

    type = CwshHistoryModifierType::REPEAT;
  }
  else if (str_[pos_] == 'q') {
    pos_++;

    type = CwshHistoryModifierType::QUOTE_WORDLIST;
  }
  else if (str_[pos_] == 'x') {
    pos_++;

    type = CwshHistoryModifierType::QUOTE_WORDS;
  }
  else if (str_[pos_] == 'r') {
    pos_++;

    type = CwshHistoryModifierType::ROOT;
  }
  else if (str_[pos_] == 'e') {
    pos_++;

    type = CwshHistoryModifierType::EXTENSION;
  }
  else if (str_[pos_] == 'h') {
    pos_++;

    type = CwshHistoryModifierType::HEADER;
  }
  else if (str_[pos_] == 't') {
    pos_++;

    type = CwshHistoryModifierType::TAIL;
  }
  else
    CWSH_THROW("Bad modifier.");

  CwshHistoryModifier modifier(type, global, old_str, new_str);

  operation_->addModifier(modifier);

  return true;
}

void
CwshHistoryParser::
parseQuickSubStr()
{
  int command_num = cwsh_->getHistoryCommandNum();

  operation_->setCommandNum (command_num - 1);
  operation_->setCommandType(CwshHistoryCommandType::QUICK_SUBSTR);

  //------

  uint len = str_.size();

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
CwshHistoryParser::
apply()
{
  std::string str = str_;

  uint num_operations = operations_.size();

  for (int i = num_operations - 1; i >= 0; i--)
    str = operations_[i]->apply(*this, str);

  if (print_) {
    std::cout << str << std::endl;

    throw CwshHistoryIgnore();
  }

  return str;
}

std::string
CwshHistoryParser::
apply(const std::vector<std::string> &words)
{
  std::string str = str_;

  uint num_operations = operations_.size();

  if (num_operations > 0) {
    for (int i = num_operations - 1; i >= 0; i--)
      str = operations_[i]->apply(*this, str, words);
  }
  else
    str += " " + CStrUtil::toString(words, 1);

  if (print_) {
    std::cout << str << std::endl;

    throw CwshHistoryIgnore();
  }

  return str;
}

void
CwshHistoryParser::
display() const
{
  uint num_operations = operations_.size();

  for (uint i = 0; i < num_operations; i++)
    operations_[i]->display(str_);
}

bool
CwshHistoryParser::
isSubStrChar(char c)
{
  if (c != '^' && c != '$' && c != '-' && c != '*' &&
      c != '#' && c != ':' && ! isspace(c))
    return true;

  return false;
}

//---------------

CwshHistoryOperation::
CwshHistoryOperation(Cwsh *cwsh) :
 cwsh_(cwsh)
{
  start_pos_     = 0;
  end_pos_       = 0;
  command_num_   = 0;
  start_arg_num_ = 0;
  end_arg_num_   = -1;
  force_args_    = false;
  command_type_  = CwshHistoryCommandType::NONE;
  old_str_       = "";
  new_str_       = "";
}

CwshHistoryOperation::
~CwshHistoryOperation()
{
}

std::string
CwshHistoryOperation::
apply(CwshHistoryParser &parser, const std::string &line)
{
  std::string lstr = line.substr(0, start_pos_);

  std::string rstr;

  uint len = line.size();

  if (end_pos_ < (int) len)
    rstr = line.substr(end_pos_);

  std::string command;

  if      (command_type_ == CwshHistoryCommandType::QUICK_SUBSTR) {
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
  else if (command_type_ == CwshHistoryCommandType::USE_RESULT)
    command = new_str_;
  else if      (command_type_ == CwshHistoryCommandType::SEARCH_START) {
    if (! cwsh_->findHistoryCommandStart(new_str_, command_num_))
      CWSH_THROW(new_str_ + ": Event not found.");

    command = cwsh_->getHistoryCommand(command_num_);
  }
  else if (command_type_ == CwshHistoryCommandType::SEARCH_IN) {
    if (! cwsh_->findHistoryCommandIn(new_str_, command_num_))
      CWSH_THROW(new_str_ + ": Event not found.");

    command = cwsh_->getHistoryCommand(command_num_);
  }
  else if (command_type_ == CwshHistoryCommandType::SEARCH_ARG) {
    int arg_num;

    if (! cwsh_->findHistoryCommandArg(new_str_, command_num_, arg_num))
      CWSH_THROW(new_str_ + ": Event not found.");

    command = cwsh_->getHistoryCommandArg(command_num_, arg_num);
  }
  else
    command = cwsh_->getHistoryCommand(command_num_);

  CStrWords words = CStrUtil::toWords(command, nullptr);

  if (start_arg_num_ < 0)
    start_arg_num_ += words.size();

  if (end_arg_num_ < 0)
    end_arg_num_ += words.size();

  if (start_arg_num_ < 0 || start_arg_num_ >= words.size() ||
      end_arg_num_   < 0 || end_arg_num_   >= words.size() ||
      start_arg_num_ > end_arg_num_) {
    if (! force_args_)
      CWSH_THROW("Bad ! arg selector.");

    command = "";
  }
  else {
    if (start_arg_num_ != 0 || end_arg_num_ != words.size() - 1)
      words.truncate(start_arg_num_, end_arg_num_);
  }

  command = CStrUtil::toString(words, " ");

  int num_modifiers = modifiers_.size();

  for (int i = 0; i < num_modifiers; i++) {
    CwshHistoryModifierType  type    = modifiers_[i].getType();
    bool                     global  = modifiers_[i].isGlobal();
    const std::string       &new_str = modifiers_[i].getNewString();
    const std::string       &old_str = modifiers_[i].getOldString();

    if      (type == CwshHistoryModifierType::PRINT)
      parser.setPrint(true);
    else if (type == CwshHistoryModifierType::SUBSTITUTE) {
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
          std::string::size_type pos1 = pos + old_str.size() - 1;

          std::string commandl = command.substr(0, pos);

          std::string commandr;

          if (pos1 + 1 < command.size())
            commandr = command.substr(pos1 + 1);

          command = commandl + new_str + commandr;

          last_pos = pos;

          pos = command.rfind(old_str);
        }
      }
    }
    else if (type == CwshHistoryModifierType::REPEAT) {
    }
    else if (type == CwshHistoryModifierType::QUOTE_WORDLIST) {
    }
    else if (type == CwshHistoryModifierType::QUOTE_WORDS) {
    }
    else if (type == CwshHistoryModifierType::ROOT) {
      CStrWords words = CStrUtil::toWords(command, nullptr);

      int num_words = words.size();

      for (int i = 0; i < num_words; i++) {
        std::string word = words[i].getWord();

        std::string::size_type pos = word.rfind('.');

        if (pos != std::string::npos)
          words.truncateWord(i, 0, pos - 1);

        if (! global)
          break;
      }

      command = CStrUtil::toString(words, " ");
    }
    else if (type == CwshHistoryModifierType::EXTENSION) {
      CStrWords words = CStrUtil::toWords(command, nullptr);

      int num_words = words.size();

      for (int i = 0; i < num_words; i++) {
        std::string word = words[i].getWord();

        std::string::size_type pos = word.rfind('.');

        if (pos != std::string::npos)
          words.truncateWord(i, pos + 1, words.size() - 1);

        if (! global)
          break;
      }

      command = CStrUtil::toString(words, " ");
    }
    else if (type == CwshHistoryModifierType::HEADER) {
      CStrWords words = CStrUtil::toWords(command, nullptr);

      int num_words = words.size();

      for (int i = 0; i < num_words; i++) {
        std::string word = words[i].getWord();

        std::string::size_type pos = word.rfind('/');

        if (pos != std::string::npos)
          words.truncateWord(i, 0, pos - 1);

        if (! global)
          break;
      }

      command = CStrUtil::toString(words, " ");
    }
    else if (type == CwshHistoryModifierType::TAIL) {
      CStrWords words = CStrUtil::toWords(command, nullptr);

      int num_words = words.size();

      for (int i = 0; i < num_words; i++) {
        std::string word = words[i].getWord();

        std::string::size_type pos = word.rfind('/');

        if (pos != std::string::npos)
          words.truncateWord(i, pos + 1, words.size() - 1);

        if (! global)
          break;
      }

      command = CStrUtil::toString(words, " ");
    }
  }

  std::string line1 = lstr + command + rstr;

  return line1;
}

std::string
CwshHistoryOperation::
apply(CwshHistoryParser &parser, const std::string &line, const std::vector<std::string> &words)
{
  int command_num = cwsh_->getHistoryCommandNum();

  if (command_num_ != command_num - 1)
    CWSH_THROW("Bad ! arg selector.");

  std::string lstr = line.substr(0, start_pos_);

  std::string rstr;

  int len = line.size();

  if (end_pos_ < len)
    rstr = line.substr(end_pos_);

  int num_words = words.size();

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

  int num_modifiers = modifiers_.size();

  for (int i = 0; i < num_modifiers; i++) {
    CwshHistoryModifierType  type    = modifiers_[i].getType();
    bool                     global  = modifiers_[i].isGlobal();
    const std::string       &new_str = modifiers_[i].getNewString();
    const std::string       &old_str = modifiers_[i].getOldString();

    if      (type == CwshHistoryModifierType::PRINT)
      parser.setPrint(true);
    else if (type == CwshHistoryModifierType::SUBSTITUTE) {
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
          std::string::size_type pos1 = pos + old_str.size() - 1;

          std::string commandl = command.substr(0, pos);

          std::string commandr;

          if (pos1 + 1 < command.size())
            commandr = command.substr(pos1 + 1);

          command = commandl + new_str + commandr;

          last_pos = pos;

          pos = command.rfind(old_str);
        }
      }
    }
    else if (type == CwshHistoryModifierType::REPEAT) {
    }
    else if (type == CwshHistoryModifierType::QUOTE_WORDLIST) {
    }
    else if (type == CwshHistoryModifierType::QUOTE_WORDS) {
    }
    else if (type == CwshHistoryModifierType::ROOT) {
      CStrWords words1 = CStrUtil::toWords(command, nullptr);

      int num_words1 = words1.size();

      for (int i = 0; i < num_words1; i++) {
        std::string word = words1[i].getWord();

        std::string::size_type pos = word.rfind('.');

        if (pos != std::string::npos)
          words1.truncateWord(i, 0, pos - 1);

        if (! global)
          break;
      }

      command = CStrUtil::toString(words1, " ");
    }
    else if (type == CwshHistoryModifierType::EXTENSION) {
      CStrWords words1 = CStrUtil::toWords(command, nullptr);

      int num_words1 = words1.size();

      for (int i = 0; i < num_words1; i++) {
        std::string word = words1[i].getWord();

        std::string::size_type pos = word.rfind('.');

        if (pos != std::string::npos)
          words1.truncateWord(i, pos + 1, words1.size() - 1);

        if (! global)
          break;
      }

      command = CStrUtil::toString(words1, " ");
    }
    else if (type == CwshHistoryModifierType::HEADER) {
      CStrWords words1 = CStrUtil::toWords(command, nullptr);

      int num_words1 = words1.size();

      for (int i = 0; i < num_words1; i++) {
        std::string word = words1[i].getWord();

        std::string::size_type pos = word.rfind('/');

        if (pos != std::string::npos)
          words1.truncateWord(i, 0, pos - 1);

        if (! global)
          break;
      }

      command = CStrUtil::toString(words1, " ");
    }
    else if (type == CwshHistoryModifierType::TAIL) {
      CStrWords words1 = CStrUtil::toWords(command, nullptr);

      int num_words1 = words1.size();

      for (int i = 0; i < num_words1; i++) {
        std::string word = words1[i].getWord();

        std::string::size_type pos = word.rfind('/');

        if (pos != std::string::npos)
          words1.truncateWord(i, pos + 1, words1.size() - 1);

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
CwshHistoryOperation::
display(const std::string &str) const
{
  std::string command = str.substr(start_pos_, end_pos_ - start_pos_);

  std::cout << "Command " << command << std::endl;

  if      (command_type_ == CwshHistoryCommandType::QUICK_SUBSTR)
    std::cout << "Replace " << old_str_ << " with " << new_str_ << std::endl;
  else if (command_type_ == CwshHistoryCommandType::USE_RESULT)
    std::cout << "Result " << new_str_ << std::endl;
  else {
    if      (command_type_ == CwshHistoryCommandType::SEARCH_START)
      std::cout << "Last Command with " << new_str_ << " at start" << std::endl;
    else if (command_type_ == CwshHistoryCommandType::SEARCH_IN)
      std::cout << "Last Command with " << new_str_ << " anywhere" << std::endl;
    else if (command_type_ == CwshHistoryCommandType::SEARCH_ARG)
      std::cout << "Last Arg with " << new_str_ << " anywhere" << std::endl;
    else
      std::cout << "Command Num " << command_num_ << std::endl;

    std::cout << "Args " << start_arg_num_ << " " << end_arg_num_ << std::endl;
  }

  int num_modifiers = modifiers_.size();

  for (int i = 0; i < num_modifiers; i++)
    modifiers_[i].print();
}

void
CwshHistoryModifier::
print() const
{
  if      (type_ == CwshHistoryModifierType::PRINT)
    std::cout << "Print" << std::endl;
  else if (type_ == CwshHistoryModifierType::SUBSTITUTE) {
    if (global_)
      std::cout << "Globally ";

    std::cout << "Substitute " << old_str_ << " with " << new_str_ << std::endl;
  }
  else if (type_ == CwshHistoryModifierType::REPEAT) {
    if (global_)
      std::cout << "Globally ";

    std::cout << "Repeat" << std::endl;
  }
  else if (type_ == CwshHistoryModifierType::QUOTE_WORDLIST)
    std::cout << "Quote Wordlist";
  else if (type_ == CwshHistoryModifierType::QUOTE_WORDS)
    std::cout << "Quote Words";
  else if (type_ == CwshHistoryModifierType::ROOT) {
    if (global_)
      std::cout << "Globally ";

    std::cout << "Root" << std::endl;
  }
  else if (type_ == CwshHistoryModifierType::EXTENSION) {
    if (global_)
      std::cout << "Globally ";

    std::cout << "Extension" << std::endl;
  }
  else if (type_ == CwshHistoryModifierType::HEADER) {
    if (global_)
      std::cout << "Globally ";

    std::cout << "Header" << std::endl;
  }
  else if (type_ == CwshHistoryModifierType::TAIL) {
    if (global_)
      std::cout << "Globally ";

    std::cout << "Tail" << std::endl;
  }
}
