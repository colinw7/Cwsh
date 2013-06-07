#include <COSTerm.h>
#include <COSProcess.h>
#include <COSErr.h>
#include <CStrUtil.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <fcntl.h>

extern "C" {
# include "termios.h"

  extern int tgetent(char *, char *);
  extern int tgetnum(char *);
};

#include "sys/ioctl.h"

#define HAS_POSIX_OPENPT 1

static const char *
color_xterm_colors[] = {
  "[0m" , /* White   */
  "[31m", /* Red     */
  "[32m", /* Green   */
  "[33m", /* Yellow  */
  "[34m", /* Blue    */
  "[35m", /* Magenta */
  "[36m", /* Cyan    */
};

static const char *
normal_xterm_colors[] = {
  "[0m", /* Normal    */
  "[4m", /* Underline */
  "[1m", /* Bold      */
  "[1m", /* Bold      */
  "[7m", /* Invert    */
  "[0m", /* Normal    */
  "[7m", /* Invert    */
};

static const char *
hp_colors[] = {
  "&v0S", /* Whitel  */
  "&v1S", /* Red     */
  "&v2S", /* Green   */
  "&v3S", /* Yellow  */
  "&v4S", /* Blue    */
  "&v5S", /* Magenta */
  "&v6S", /* Cyan    */
};

const char **COSTerm::colors_ = NULL;

std::string
COSTerm::
getTerminalName(int fd)
{
  char *tty_name = ttyname(fd);

  if (tty_name == NULL)
    return "";

  return tty_name;
}

std::string
COSTerm::
getTerminalPath()
{
  char *tty_name = ctermid(NULL);

  if (tty_name == NULL)
    return "";

  return tty_name;
}

pid_t
COSTerm::
getTerminalProcessGroupId(int fd)
{
  std::string tty_name = getTerminalName(fd);

  return getTerminalProcessGroupId(tty_name);
}

pid_t
COSTerm::
getTerminalProcessGroupId(const std::string &term)
{
  int fd = open(term.c_str(), O_RDWR);

  if (fd < 0) {
    fd = open(term.c_str(), O_RDONLY);

    if (fd < 0)
      return -1;
  }

  pid_t id;

#ifdef NEVER
  int error = ioctl(fd, TIOCGPGRP, (char *) &id);
#else
  id = tcgetpgrp(fd);
#endif

  close(fd);

  return id;
}

pid_t
COSTerm::
getTerminalSessionId(int fd)
{
  int tfd = getTerminalId(fd);

  if (tfd < 0) return -1;

#ifdef COS_HAS_TCGETSID
  pid_t id = tcgetsid(tfd);
#else
  pid_t id = COSProcess::getSessionId(tcgetpgrp(tfd));
#endif

  close(fd);

  return id;
}

int
COSTerm::
getTerminalId(int fd)
{
  std::string tty_name = getTerminalName(fd);

  int tfd = open(tty_name.c_str(), O_RDWR);

  if (tfd < 0) {
    tfd = open(tty_name.c_str(), O_RDONLY);

    if (tfd < 0)
      return -1;
  }

  return tfd;
}

bool
COSTerm::
getCharSize(int *rows, int *cols)
{
  if (isatty(STDIN_FILENO)) {
    if (getCharSize(STDIN_FILENO, rows, cols))
      return true;
  }

#ifdef CURSES
  WINDOW *w;

  use_env(false);

  w = initscr();

  endwin();

  *rows = LINES;
  *cols = COLS;
#else
  *rows = 60;
  *cols = 0;

  char *columns;

  if ((columns = getenv("COLUMNS")) != NULL)
    *cols = atoi(columns);

  if (*cols == 0)
    *cols = COSTerm::getNumColumns();

  if (*cols == 0)
    *cols = 80;
#endif

  return false;
}

bool
COSTerm::
getCharSize(int fd, int *rows, int *cols)
{
  struct winsize ts;

  if (ioctl(fd, TIOCGWINSZ, &ts) == -1)
    return false;

  *rows = ts.ws_row;
  *cols = ts.ws_col;

  return true;
}

bool
COSTerm::
setCharSize(int fd, int rows, int cols)
{
  struct winsize ts;

  if (ioctl(fd, TIOCGWINSZ, &ts) == -1)
    return false;

  if (ts.ws_col != 0)
    ts.ws_xpixel = cols*(ts.ws_xpixel/ts.ws_col);
  if (ts.ws_row != 0)
    ts.ws_ypixel = rows*(ts.ws_ypixel/ts.ws_row);

  ts.ws_row = rows;
  ts.ws_col = cols;

  ioctl(fd, TIOCSWINSZ, (char *) &ts);

  return true;
}

bool
COSTerm::
getPixelSize(int *width, int *height)
{
  int fd = COSTerm::getTerminalId();

  if (fd < 0)
    return false;

  return getPixelSize(fd, width, height);
}

bool
COSTerm::
getPixelSize(int fd, int *width, int *height)
{
  struct winsize ts;

  if (ioctl(fd, TIOCGWINSZ, &ts) == -1)
    return false;

  *width  = ts.ws_xpixel;
  *height = ts.ws_ypixel;

  return true;
}

bool
COSTerm::
setPixelSize(int fd, int width, int height)
{
  struct winsize ts;

  if (ioctl(fd, TIOCGWINSZ, &ts) == -1)
    return false;

  if (ts.ws_xpixel != 0)
    ts.ws_row = (width *ts.ws_col)/ts.ws_xpixel;
  if (ts.ws_ypixel != 0)
    ts.ws_col = (height*ts.ws_row)/ts.ws_ypixel;

  ts.ws_xpixel = width;
  ts.ws_ypixel = height;

  ioctl(fd, TIOCSWINSZ, (char *) &ts);

  return true;
}

int
COSTerm::
getNumColumns()
{
  static char term_buffer[2048];

  const char *term;

  if ((term = getenv("TERM")) == NULL)
    term = "vt100";

  int no = tgetent(term_buffer, (char *) term);

  if (no <= 0)
    return 0;

  int cols = tgetnum((char *) "cols");

  return cols;
}

const char *
COSTerm::
getColorStr(int color)
{
  if (color < 0 || color > 6)
    return "";

  const char **colors = getColorStrs();

  return colors[color];
}

const char **
COSTerm::
getColorStrs()
{
  if (colors_ == NULL) {
    const char *term;

    if ((term = getenv("TERM")) == NULL)
      term = "vt100";

    if      (strcmp(term, "xterm"      ) == 0 ||
             strcmp(term, "xterm-color") == 0) {
      bool colored = true;

      char *color_xterm;

      if ((color_xterm = getenv( "COLOR_XTERM")) == NULL &&
          (color_xterm = getenv("COLOUR_XTERM")) == NULL)
        colored = false;

      if (colored)
        colors_ = color_xterm_colors;
      else
        colors_ = normal_xterm_colors;
    }
    else if (strncmp(term, "hp", 2) == 0)
      colors_ = hp_colors;
    else
      colors_ = normal_xterm_colors;
  }

  return colors_;
}

bool
COSTerm::
cgets(std::string &buf)
{
  FILE *tty;

  std::string tty_name = COSTerm::getTerminalName(STDIN_FILENO);

  if (! (tty = fopen(tty_name.c_str(), "w+")))
    return false;

  /* get attributes on the input tty and set them to new settings */

  struct termios init, newsettings;

  tcgetattr(fileno(stdin), &init);

  newsettings              = init;
  newsettings.c_lflag     &= ~(ICANON | ECHO);
  newsettings.c_cc[VMIN ]  = 1;
  newsettings.c_cc[VTIME]  = 0;

  if (tcsetattr(fileno(stdin), TCSANOW, &newsettings) != 0)
    return -1;

  int c;

  for (;;) {
    c = getc(stdin);

    if ((c == '\n') || (c == '\r') || (c == EOF))
      break;

    if ((c == '\b') || (c == 0x7f)) {
      if (buf.size() > 0) {
        buf = buf.substr(0, buf.size() - 1);

        fputs("\b \b", stdout);
      }
      else
        putc('\a', stdout);

      continue;
    }

    fputc(c, stdout);

    buf += char(c);
  }

  tcsetattr(fileno(tty), TCSANOW, &init);

  fflush(tty);
  fclose(tty);

  printf("\n");

  return true;
}

//-------

bool
COSTerm::
openMasterPty(int *fd, std::string &slaveName)
{
  // Open Master Pseudo Terminal

#if HAS_POSIX_OPENPT
  *fd = posix_openpt(O_RDWR | O_NOCTTY);
#elif HAS_GETPT
  *fd = getpt();
#elif HAS_PTMX
  *fd = open("/dev/ptmx", O_RDWR | O_NOCTTY);
#elif BSD_TERM
  // TODO: need new pty name here. For BSD this will by names:
  // /dev/pty<X><Y> where <X> is [p-za-e] and <Y> is [0-9a-f]
  static char xchars[]   = "pqrstuvwxyzabcde";
  static uint num_xchars = sizeof(xchars);
  static char ychars[]   = "0123456789abcdef";
  static uint num_ychars = sizeof(ychars);

  static char bsd_pty[] = "/dev/ptyxy";
  static char bsd_tty[] = "/dev/ttyxy";

  for (uint x = 0; x < num_xchars; ++x) {
    bsd_pty[8] = xchars[x];
    bsd_tty[8] = xchars[x];

    for (uint y = 0; y < num_ychars; ++y) {
      bsd_pty[9] = ychars[y];
      bsd_tty[9] = ychars[y];

      *fd = open(bsd_pty, O_RDWR | O_NOCTTY);

      if (*fd == -1) {
        if (errno == ENOENT) break;

        continue;
      }

      if (*fd >= 0) break;
    }
  }
#else
  return false;
#endif

  if (*fd == -1)
    return false;

  //----

#ifndef BSD_TERM
  // Ensure Terminal has process permission (needed to make portable)

  if (grantpt(*fd) == -1) {
    closeNoError(*fd); *fd = -1;
    return false;
  }

  //----

  // Finished setup so we now must unlock slave to use

  if (unlockpt(*fd) == -1) {
    closeNoError(*fd); *fd = -1;
    return false;
  }

  //----

  char *ptr = ptsname(*fd);

  if (ptr == NULL) {
    closeNoError(*fd); *fd = -1;
    return false;
  }

  slaveName = ptr;
#else
  slaveName = bsd_tty;
#endif

  return true;
}

bool
COSTerm::
ptyFork(int *mfd, std::string &slaveName, pid_t *childPid)
{
  *mfd      = 0;
  *childPid = 0;

  if (! openMasterPty(mfd, slaveName))
    return false;

  *childPid = fork();

  if (*childPid == -1) { // failed
    closeNoError(*mfd);
    return false;
  }

  // ---- parent ----

  if (*childPid != 0)
    return true;

  // ---- child ----

  // master pty not needed
  close(*mfd);

  // start new session (child loses controlling terminal)
  if (setsid() == -1)
    COSErr::exit("ptyFork: setsid");

  // Becomes controlling tty
  int sfd = open(slaveName.c_str(), O_RDWR);

  if (sfd == -1)
   COSErr::exit("ptyFork: open-slave");

#ifdef TIOCSCTTY
  // Acquire controlling tty on BSD
  if (ioctl(sfd, TIOCSCTTY, 0) == -1)
    COSErr::exit("ptyFork:ioctl-TIOCSCTTY");
#endif

  // Duplicate pty slave to be child's stdin, stdout, and stderr

  if (dup2(sfd, STDIN_FILENO ) != STDIN_FILENO ) COSErr::exit("ptyFork:dup2-STDIN_FILENO");
  if (dup2(sfd, STDOUT_FILENO) != STDOUT_FILENO) COSErr::exit("ptyFork:dup2-STDOUT_FILENO");
  if (dup2(sfd, STDERR_FILENO) != STDERR_FILENO) COSErr::exit("ptyFork:dup2-STDERR_FILENO");

  // No longer need pty slave (don't close if equal to stdin, stdout or stderr
  if (sfd > STDERR_FILENO)
    closeNoError(sfd);

  return true;
}

//-------

void
COSTerm::
closeNoError(int fd)
{
  int saveErrno = errno;

  (void) close(fd);

  errno = saveErrno;
}
