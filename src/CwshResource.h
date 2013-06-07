class CwshResource {
 private:
  static CwshResourceLimit limits_[];

 public:
  CwshResource();

  void limit(const std::string &name, const std::string &value, bool hard=false);
  void unlimitAll();
  void unlimit(const std::string &name);
  void printAll(bool hard=false);
  void print(const std::string &name, bool hard=false);

 private:
  void unlimit(int id);

  void print(CwshResourceLimit *rlimit, bool hard);

  int convertValue(CwshResourceLimit *rlimit, const std::string &value);

  CwshResourceLimit *getLimit(const std::string &name);
};
