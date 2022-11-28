#ifndef CWSH_DIR_STACK_H
#define CWSH_DIR_STACK_H

namespace Cwsh {

class DirStack {
 public:
  DirStack();

  void push();
  void push(const std::string &dirname);

  std::string pop();
  std::string pop(int pos);

  int size();

  void print(std::ostream &os = std::cout);

 private:
  std::vector<std::string> dirStack_;
};

}

#endif
