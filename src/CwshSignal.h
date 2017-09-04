enum class CwshInterruptType {
  NORMAL,
  IGNORE,
  GOTO
};

class CwshSignal {
 public:
  static void addHandlers();

  static void nohup();

  static CwshSignal *lookup(const std::string &name);
  static CwshSignal *lookup(int num);

  static int         getNumSignals();
  static CwshSignal *getSignal(int i);

  static void interruptReset() {
    interrupt_type_ = CwshInterruptType::NORMAL;
  }

  static void interruptIgnore() {
    interrupt_type_ = CwshInterruptType::IGNORE;
  }

  static void interruptGoto(const std::string &label) {
    interrupt_type_  = CwshInterruptType::GOTO;
    interrupt_label_ = label;
  }

  const std::string &getName() const { return name_; }
  int                getNum () const { return num_ ; }

 private:
  static void termHandler(int sig);
  static void interruptHandler(int sig);
  static void stopHandler(int sig);
  static void genericHandler(int sig);

  CwshSignal(const std::string &name, int num);

 private:
  std::string name_;
  int         num_;

  static CwshSignal signals_[];

  static CwshInterruptType interrupt_type_;
  static std::string       interrupt_label_;
};
