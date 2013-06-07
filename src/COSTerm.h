#ifndef COS_TERM_H
#define COS_TERM_H

#include <string>

class COSTerm {
 private:
  static const char **colors_;

 public:
  static std::string getTerminalName(int fd=0);

  static std::string getTerminalPath();

  static pid_t getTerminalProcessGroupId(int fd=0);

  static pid_t getTerminalProcessGroupId(const std::string &term);

  static pid_t getTerminalSessionId(int fd=0);

  static int getTerminalId(int fd=0);

  static bool getCharSize(int *rows, int *cols);
  static bool getCharSize(int fd, int *rows, int *cols);
  static bool setCharSize(int fd, int rows, int cols);

  static bool getPixelSize(int *width, int *height);
  static bool getPixelSize(int fd, int *width, int *height);
  static bool setPixelSize(int fd, int width, int height);

  static const char  *getColorStr(int color);
  static const char **getColorStrs();

  static bool cgets(std::string &buf);

  static bool openMasterPty(int *fd, std::string &slaveName);

  static bool ptyFork(int *mfd, std::string &slaveName, pid_t *childPid);

  static void closeNoError(int fd);

 private:
  static int getNumColumns();
};

#endif
