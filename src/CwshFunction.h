#ifndef CWSH_FUNCTION_H
#define CWSH_FUNCTION_H

namespace Cwsh {

class FunctionMgr {
 public:
  FunctionMgr(App *cwsh);

  Function *define(const std::string &name, const LineArray &lines);

  void undefine(const std::string &name);

  Function *lookup(const std::string &name);

  void listAll(bool all);

 private:
  using FunctionP    = std::shared_ptr<Function>;
  using FunctionList = std::map<std::string, FunctionP>;

  CPtr<App>    cwsh_;
  FunctionList functionList_;
};

//---

class Function {
 public:
  Function(App *cwsh, const std::string &name, const LineArray &lines);
 ~Function();

  static void runProc(const ArgArray &args, CCommand::CallbackData data);

  void run(const ArgArray &args);

  const std::string &getName() const { return name_; }

  const std::string &getFilename() const { return filename_; }
  void setFilename(const std::string &v) { filename_ = v; }

  int getLineNum() const { return lineNum_; }
  void setLineNum(int i) { lineNum_ = i; }

  void list(bool all);

 private:
  CPtr<App>   cwsh_;
  std::string name_;
  LineArray   lines_;
  std::string filename_;
  int         lineNum_ { -1 };
};

}
#endif
