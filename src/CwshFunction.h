#ifndef CWSH_FUNCTION_H
#define CWSH_FUNCTION_H

class CwshFunctionMgr {
 public:
  CwshFunctionMgr(Cwsh *cwsh);

  CwshFunction *define(const CwshFunctionName &name, const CwshLineArray &lines);

  void undefine(const CwshFunctionName &name);

  CwshFunction *lookup(const CwshFunctionName &name);

  void listAll(bool all);

 private:
  typedef CAutoPtrMap<CwshFunctionName,CwshFunction> FunctionList;

  CPtr<Cwsh>   cwsh_;
  FunctionList function_list_;
};

//---

class CwshFunction {
 public:
  CwshFunction(Cwsh *cwsh, const CwshFunctionName &name, const CwshLineArray &lines);
 ~CwshFunction();

  static void runProc(const CwshArgArray &args, CCommand::CallbackData data);

  void run(const CwshArgArray &args);

  const CwshFunctionName &getName() const { return name_; }

  const std::string &getFilename() const { return filename_; }
  void setFilename(const std::string &v) { filename_ = v; }

  int getLineNum() const { return lineNum_; }
  void setLineNum(int i) { lineNum_ = i; }

  void list(bool all);

 private:
  CPtr<Cwsh>       cwsh_;
  CwshFunctionName name_;
  CwshLineArray    lines_;
  std::string      filename_;
  int              lineNum_ { -1 };
};

#endif
