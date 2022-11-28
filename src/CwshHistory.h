#ifndef CWSH_HISTORY_H
#define CWSH_HISTORY_H

#include <CHistory.h>

namespace Cwsh {

struct HistoryIgnore {
  void *dummy { nullptr };
};

//---

class History {
 public:
  History(App *cwsh);
 ~History();

  int getCommandNum() const { return commandNum_; }

  bool findCommandStart(const std::string &text, int &commandNum);
  bool findCommandIn(const std::string &text, int &commandNum);
  bool findCommandArg(const std::string &text, int &commandNum, int &argNum);

  std::string getCommand(int num);
  std::string getCommandArg(int num, int argNum);

  void addFile(const std::string &filename);

  void addCommand(const std::string &text);

  void setCurrent(const std::string &text);

  void display(int num, bool showNumbers, bool showTime, bool reverse) const;

  bool hasPrevCommand();
  bool hasNextCommand();

  std::string getPrevCommand();
  std::string getNextCommand();

  std::string getPath();

  std::string getHistoryMsg() const;

 private:
  int getSize() const;
  int getSaveSize() const;

  void updateSize() const;

  static std::string getFilename();

 private:
  CPtr<App> cwsh_;
  CHistory  history1_;
  CHistory  history_;
  int       commandNum_ { 0 };
};

}

#endif
