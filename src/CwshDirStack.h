#ifndef CWSH_DIR_STACK_H
#define CWSH_DIR_STACK_H

class CwshDirStack {
 public:
  CwshDirStack();

  void push();
  void push(const std::string &dirname);

  std::string pop();
  std::string pop(int pos);

  int size();

  void print(std::ostream &os = std::cout);

 private:
  std::vector<std::string> dir_stack_;
};

#endif
