namespace Cwsh {

enum class InterruptType {
  NORMAL,
  IGNORE,
  GOTO
};

class Signal {
 public:
  static void addHandlers();

  static void nohup();

  static Signal *lookup(const std::string &name);
  static Signal *lookup(int num);

  static int     getNumSignals();
  static Signal *getSignal(int i);

  static void interruptReset () { interruptType_ = InterruptType::NORMAL; }
  static void interruptIgnore() { interruptType_ = InterruptType::IGNORE; }

  static void interruptGoto(const std::string &label) {
    interruptType_  = InterruptType::GOTO;
    interruptLabel_ = label;
  }

  const std::string &getName() const { return name_; }
  int                getNum () const { return num_ ; }

 private:
  static void termHandler(int sig);
  static void interruptHandler(int sig);
  static void stopHandler(int sig);
  static void genericHandler(int sig);

  Signal(const std::string &name, int num);

 private:
  std::string name_;
  int         num_;

  static Signal signals_[];

  static InterruptType interruptType_;
  static std::string   interruptLabel_;
};

}
