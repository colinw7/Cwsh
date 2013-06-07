#ifndef CWSH_HISTORY_H
#define CWSH_HISTORY_H

#include <CHistory.h>

struct CwshHistoryIgnore {
  void *dummy;
};

class CwshHistory {
 public:
  CwshHistory(Cwsh *cwsh);
 ~CwshHistory();

  int getCommandNum() const { return command_num_; }

  bool findCommandStart(const std::string &text, int &command_num);
  bool findCommandIn(const std::string &text, int &command_num);
  bool findCommandArg(const std::string &text, int &command_num, int &arg_num);

  std::string getCommand(int num);
  std::string getCommandArg(int num, int arg_num);

  void addFile(const std::string &filename);

  void addCommand(const std::string &text);

  void setCurrent(const std::string &text);

  void display(int num, bool show_numbers, bool show_time, bool reverse);

  bool hasPrevCommand();
  bool hasNextCommand();

  std::string getPrevCommand();
  std::string getNextCommand();

  std::string getPath();

  std::string getHistoryMsg() const;

 private:
  int getSize() const;
  int getSaveSize() const;

  void updateSize();

  static std::string getFilename();

 private:
  CPtr<Cwsh> cwsh_;
  CHistory   history1_;
  CHistory   history_;
  int        command_num_;
};

#endif
