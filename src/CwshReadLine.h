#ifndef CWSH_READ_LINE_H
#define CWSH_READ_LINE_H

#include <CReadLine.h>

class CwshReadLine : public CReadLine {
 public:
  CwshReadLine(Cwsh *cwsh);

  std::string readLine();
  void        beep();
  void        interrupt();
  void        timeout();

 private:
  bool completeLine(const std::string &line, std::string &line1);
  bool showComplete(const std::string &line);
  bool getPrevCommand(std::string &line);
  bool getNextCommand(std::string &line);

 private:
  CPtr<Cwsh> cwsh_;
};

#endif
