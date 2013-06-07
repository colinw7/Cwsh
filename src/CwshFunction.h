#ifndef CWSH_FUNCTION_H
#define CWSH_FUNCTION_H

class CwshFunctionMgr {
 public:
  CwshFunctionMgr(Cwsh *cwsh);

  void define  (const CwshFunctionName &name, const CwshLineArray &lines);
  void undefine(const CwshFunctionName &name);

  CwshFunction *lookup(const CwshFunctionName &name);

  void listAll();

 private:
  typedef CAutoPtrMap<CwshFunctionName,CwshFunction> FunctionList;

  CPtr<Cwsh>   cwsh_;
  FunctionList function_list_;
};

class CwshFunction {
 public:
  CwshFunction(Cwsh *cwsh, const CwshFunctionName &name, const CwshLineArray &lines);
 ~CwshFunction();

  static void runProc(const CwshArgArray &args, CCommand::CallbackData data);

  void run(const CwshArgArray &args);

  const CwshFunctionName &getName() const { return name_; }

 private:
  CPtr<Cwsh>       cwsh_;
  CwshFunctionName name_;
  CwshLineArray    lines_;
};

#endif
