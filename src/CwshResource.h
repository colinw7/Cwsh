namespace Cwsh {

class Resource {
 private:
  static ResourceLimit limits_[];

 public:
  Resource();

  void limit(const std::string &name, const std::string &value, bool hard=false);
  void unlimitAll();
  void unlimit(const std::string &name);
  void printAll(bool hard=false);
  void print(const std::string &name, bool hard=false);

 private:
  void unlimit(int id);

  void print(ResourceLimit *rlimit, bool hard);

  int convertValue(ResourceLimit *rlimit, const std::string &value);

  ResourceLimit *getLimit(const std::string &name);
};

}
