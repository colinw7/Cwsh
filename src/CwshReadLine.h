#ifndef CWSH_READ_LINE_H
#define CWSH_READ_LINE_H

#include <CReadLine.h>

namespace Cwsh {

class ReadLine : public CReadLine {
 public:
  ReadLine(App *cwsh);

  std::string readLine();

  void beep() override;
  void interrupt() override;
  void timeout() override;

 private:
  bool completeLine(const std::string &line, std::string &line1) override;

  bool showComplete(const std::string &line) override;

  bool getPrevCommand(std::string &line) override;
  bool getNextCommand(std::string &line) override;

 private:
  CPtr<App> cwsh_;
};

}

#endif
