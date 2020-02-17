#ifndef CESCAPE_TYPE_H
#define CESCAPE_TYPE_H

enum class CEscapeColor {
  BLACK       = 0,
  RED         = 1,
  GREEN       = 2,
  YELLOW      = 3,
  BLUE        = 4,
  MAGENTA     = 5,
  CYAN        = 6,
  WHITE       = 7,

  BG          = 8,
  FG          = 9,

  ALT_BLACK   = 10,
  ALT_RED     = 11,
  ALT_GREEN   = 12,
  ALT_YELLOW  = 13,
  ALT_BLUE    = 14,
  ALT_MAGENTA = 15,
  ALT_CYAN    = 16,
  ALT_WHITE   = 17,

  DIM         = (1<<5),

  DIM_BLACK   = (DIM | BLACK),
  DIM_RED     = (DIM | RED),
  DIM_GREEN   = (DIM | GREEN),
  DIM_YELLOW  = (DIM | YELLOW),
  DIM_BLUE    = (DIM | BLUE),
  DIM_MAGENTA = (DIM | MAGENTA),
  DIM_CYAN    = (DIM | CYAN),
  DIM_WHITE   = (DIM | WHITE)
};

enum class CEscapeLineStyle {
  SOLID,
  DOTTED,
  DOT_DASH,
  SHORT_DASH,
  LONG_DASH
};

#endif
