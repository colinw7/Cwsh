#include <CEscape.h>
#include <CFile.h>
#include <CDir.h>
#include <COSRead.h>
#include <CStrUtil.h>

using std::string;
using std::vector;
using std::ostream;
using std::cerr;
using std::endl;

#define CTRL_A_s ""
#define CTRL_B_s ""
#define CTRL_C_s ""
#define CTRL_D_s ""
#define CTRL_E_s ""
#define CTRL_F_s ""
#define CTRL_G_s ""
#define CTRL_H_s ""
#define CTRL_I_s "\011"
#define CTRL_J_s "\012"
#define CTRL_K_s ""
#define CTRL_L_s ""
#define CTRL_M_s "\015"
#define CTRL_N_s ""
#define CTRL_O_s ""
#define CTRL_P_s ""
#define CTRL_Q_s "\021"
#define CTRL_R_s ""
#define CTRL_S_s "\023"
#define CTRL_T_s ""
#define CTRL_U_s ""
#define CTRL_V_s ""
#define CTRL_W_s ""
#define CTRL_X_s ""
#define CTRL_Y_s ""
#define CTRL_Z_s ""

#define ESC_c ''
#define ESC_s ""
#define CSI_s "["
#define OSC_s "]"
#define ST_s  "\\"

string
CEscape::
NUL()
{
  return "\000";
}

string
CEscape::
SOH()
{
  return CTRL_A_s;
}

string
CEscape::
STX()
{
  return CTRL_B_s;
}

string
CEscape::
ETX()
{
  return CTRL_C_s;
}

string
CEscape::
EOT()
{
  return CTRL_D_s;
}

string
CEscape::
ENQ()
{
  return CTRL_E_s;
}

string
CEscape::
ACK()
{
  return CTRL_F_s;
}

string
CEscape::
BEL()
{
  return CTRL_G_s;
}

string
CEscape::
BS()
{
  return CTRL_H_s;
}

string
CEscape::
HT()
{
  return CTRL_I_s;
}

string
CEscape::
LF()
{
  return CTRL_J_s;
}

string
CEscape::
VT()
{
  return CTRL_K_s;
}

string
CEscape::
FF()
{
  return CTRL_L_s;
}

string
CEscape::
CR()
{
  return CTRL_M_s;
}

string
CEscape::
SO()
{
  return CTRL_N_s;
}

string
CEscape::
SI()
{
  return CTRL_O_s;
}

string
CEscape::
DLE()
{
  return CTRL_P_s;
}

string
CEscape::
DC1()
{
  return CTRL_Q_s;
}

string
CEscape::
DC2()
{
  return CTRL_R_s;
}

string
CEscape::
DC3()
{
  return CTRL_S_s;
}

string
CEscape::
DC4()
{
  return CTRL_T_s;
}

string
CEscape::
NAK()
{
  return CTRL_U_s;
}

string
CEscape::
SYN()
{
  return CTRL_V_s;
}

string
CEscape::
ETB()
{
  return CTRL_W_s;
}

string
CEscape::
CAN()
{
  return CTRL_X_s;
}

string
CEscape::
EM()
{
  return CTRL_Y_s;
}

string
CEscape::
SUB()
{
  return CTRL_Z_s;
}

string
CEscape::
DEL()
{
  static char c[2] = { 0x7f, 0x00 };

  return &c[0];
}

string
CEscape::
SP()
{
  return " ";
}

string
CEscape::
IND()
{
  return ESC_s "D";
}

string
CEscape::
NEL()
{
  return ESC_s "E";
}

string
CEscape::
HTS()
{
  return ESC_s "H";
}

string
CEscape::
RI()
{
  return ESC_s "M";
}

string
CEscape::
SS2()
{
  return ESC_s "N";
}

string
CEscape::
SS3()
{
  return ESC_s "O";
}

string
CEscape::
DCS()
{
  return ESC_s "P";
}

string
CEscape::
SPA()
{
  return ESC_s "V";
}

string
CEscape::
EPA()
{
  return ESC_s "W";
}

string
CEscape::
SOS()
{
  return ESC_s "X";
}

string
CEscape::
DECID()
{
  return ESC_s "Z";
}

//----------------

// CSI 1t : de-iconify
string
CEscape::
windowOpDeiconify()
{
  return windowOp(WINDOW_OP_DEICONIFY);
}

// CSI 2t : iconify
string
CEscape::
windowOpIconify()
{
  return windowOp(WINDOW_OP_ICONIFY);
}

// CSI 3;<x>;<y>t : move to x,y
string
CEscape::
windowOpMove(int x, int y)
{
  return windowOp(WINDOW_OP_MOVE, CStrUtil::toString(x), CStrUtil::toString(y));
}

// CSI 4;<h>;<w>t : resize to h,w pixels
string
CEscape::
windowOpPixelSize(int h, int w)
{
  return windowOp(WINDOW_OP_PIXEL_SIZE, CStrUtil::toString(h), CStrUtil::toString(w));
}

// CSI 5t : raise
string
CEscape::
windowOpRaise()
{
  return windowOp(WINDOW_OP_RAISE);
}

// CSI 6t : lower
string
CEscape::
windowOpLower()
{
  return windowOp(WINDOW_OP_LOWER);
}

// CSI 7t : refresh
string
CEscape::
windowOpRefresh()
{
  return windowOp(WINDOW_OP_REFRESH);
}

// CSI 8;<h>;<w>t : resize to h,w chars
string
CEscape::
windowOpCharSize(int h, int w)
{
  return windowOp(WINDOW_OP_CHAR_SIZE, CStrUtil::toString(h), CStrUtil::toString(w));
}

// CSI 9;0t : restore maximized window
string
CEscape::
windowOpRestoreMaximized()
{
  return windowOp(WINDOW_OP_MAXIMIZE, "0");
}

// CSI 9;1t : maximize window
string
CEscape::
windowOpMaximize()
{
  return windowOp(WINDOW_OP_MAXIMIZE, "1");
}

// CSI 11t : report state as CSI 1 or CSI 2
string
CEscape::
windowOpReportState()
{
  return windowOp(WINDOW_OP_REPORT_STATE);
}

// CSI 13t : report pos as CSI 3;<x>;<y>t
string
CEscape::
windowOpReportPos()
{
  return windowOp(WINDOW_OP_REPORT_POS);
}

// CSI 14t : report size as CSI 4;<h>;<w>t
string
CEscape::
windowOpReportPixelSize()
{
  return windowOp(WINDOW_OP_REPORT_PIXEL_SIZE);
}

// CSI 18t : report size as CSI 8;<h>;<w>t
string
CEscape::
windowOpReportCharSize()
{
  return windowOp(WINDOW_OP_REPORT_CHAR_SIZE);
}

// CSI 19t : report screen size as CSI 9;<h>;<w>t
string
CEscape::
windowOpReportScreenSize()
{
  return windowOp(WINDOW_OP_REPORT_SCREEN_SIZE);
}

// CSI 20t : report icon label as OSC L label ST
string
CEscape::
windowOpReportIconLabel()
{
  return windowOp(WINDOW_OP_REPORT_ICON_STR);
}

// CSI 21t : report window title as OSC l label ST
string
CEscape::
windowOpReportWindowTitle()
{
  return windowOp(WINDOW_OP_REPORT_TITLE_STR);
}

// CSI 24 ... : resize to 24 or greater lines
string
CEscape::
windowOpResizeNLines(int n)
{
  return windowOp((WindowOp) n);
}

// CSI <op>;<arg1>;<arg2>t : window_op <op>
string
CEscape::
windowOp(WindowOp op, const string &arg1, const string &arg2)
{
  string str = CStrUtil::toString(int(op));

  if (! arg1.empty()) {
    str += ";" + arg1;

    if (! arg2.empty())
      str += ";" + arg2;
  }

  str += "t";

  return CSI(str);
}

bool
CEscape::
decodeWindowOp(const string &str, WindowOp &op, string &arg1, string &arg2)
{
  int len = str.size();

  if (len < 3) return false;

  if (str[0] != ESC_c || str[1] != '[' || str[len - 1] != 't')
    return false;

  int i = 2;

  vector<string> args;

  while (i < len && str[i] != 't') {
    string arg;

    while (i < len && str[i] != ';' && str[i] != 't')
      arg += str[i++];

    if (i < len && str[i] == ';')
      ++i;

    args.push_back(arg);
  }

  uint num_args = args.size();

  if (num_args < 1)
    return false;

  int op_num;

  if (! CStrUtil::toInteger(args[0], &op_num))
    return false;

  op = (WindowOp) op_num;

  if (i != len - 1)
    return false;

  arg1 = (num_args >= 2 ? args[1] : "");
  arg2 = (num_args >= 3 ? args[2] : "");

  return true;
}

string
CEscape::
oscIconWindowTitle(const string &str)
{
  return OSC_s "0;" + str + ST_s;
}

string
CEscape::
oscIconTitle(const string &str)
{
  return OSC_s "1;" + str + ST_s;
}

string
CEscape::
oscWindowTitle(const string &str)
{
  return OSC_s "2;" + str + ST_s;
}

string
CEscape::
oscWindowProp(const string &str)
{
  return OSC_s "3;" + str + ST_s;
}

string
CEscape::
oscColor(int num, const string &str)
{
  return OSC_s "4;" + CStrUtil::toString(num) + ";" + str + ST_s;
}

string
CEscape::
oscFg(const string &str)
{
  return OSC_s "10;" + str + ST_s;
}

string
CEscape::
oscBg(const string &str)
{
  return OSC_s "11;" + str + ST_s;
}

string
CEscape::
oscCursorColor(const string &str)
{
  return OSC_s "12;" + str + ST_s;
}

string
CEscape::
oscFont(const string &str)
{
  return OSC_s "50;" + str + ST_s;
}

//-----------

string
CEscape::
DECSC()
{
  return ESC_s "7";
}

string
CEscape::
DECRC()
{
  return ESC_s "8";
}

string
CEscape::
DECPAM()
{
  return ESC_s "=";
}

string
CEscape::
DECPNM()
{
  return ESC_s ">";
}

string
CEscape::
RIS()
{
  return ESC_s "c";
}

string
CEscape::
DECALN()
{
  return ESC_s "#8";
}

string
CEscape::
ICH(int n)
{
  if (n < 0)
    return CSI_s "@";
  else
    return CSI_s + CStrUtil::toString(n) + "@";
}

string
CEscape::
CUU(int n)
{
  if (n < 0)
    return CSI_s "A";
  else
    return CSI_s + CStrUtil::toString(n) + "A";
}

string
CEscape::
CUD(int n)
{
  if (n < 0)
    return CSI_s "B";
  else
    return CSI_s + CStrUtil::toString(n) + "B";
}

string
CEscape::
CUF(int n)
{
  if (n < 0)
    return CSI_s "C";
  else
    return CSI_s + CStrUtil::toString(n) + "C";
}

string
CEscape::
CUB(int n)
{
  if (n < 0)
    return CSI_s "D";
  else
    return CSI_s + CStrUtil::toString(n) + "D";
}

string
CEscape::
CNL(int n)
{
  if (n < 0)
    return CSI_s "E";
  else
    return CSI_s + CStrUtil::toString(n) + "E";
}

string
CEscape::
CPL(int n)
{
  if (n < 0)
    return CSI_s "F";
  else
    return CSI_s + CStrUtil::toString(n) + "F";
}

string
CEscape::
CHA(int n)
{
  if (n < 0)
    return CSI_s "G";
  else
    return CSI_s + CStrUtil::toString(n) + "G";
}

string
CEscape::
CUP(int row, int col)
{
  if (row < 0 || col < 0)
    return CSI_s "H";
  else
    return CSI_s + CStrUtil::toString(row) + ";" +
                   CStrUtil::toString(col) + "H";
}

string
CEscape::
CHT(int n)
{
  if (n < 0)
    return CSI_s "I";
  else
    return CSI_s + CStrUtil::toString(n) + "I";
}

string
CEscape::
ED(int n)
{
  if (n < 0)
    return CSI_s "J";
  else
    return CSI_s + CStrUtil::toString(n) + "J";
}

string
CEscape::
DECSED(int n)
{
  if (n < 0)
    return CSI_s "?J";
  else
    return CSI_s "?" + CStrUtil::toString(n) + "J";
}

string
CEscape::
EL(int n)
{
  if (n < 0)
    return CSI_s "K";
  else
    return CSI_s + CStrUtil::toString(n) + "K";
}

string
CEscape::
DECSEL(int n)
{
  if (n < 0)
    return CSI_s "?K";
  else
    return CSI_s "?" + CStrUtil::toString(n) + "K";
}

string
CEscape::
IL(int n)
{
  if (n < 0)
    return CSI_s "L";
  else
    return CSI_s + CStrUtil::toString(n) + "L";
}

string
CEscape::
DL(int n)
{
  if (n < 0)
    return CSI_s "M";
  else
    return CSI_s + CStrUtil::toString(n) + "M";
}

string
CEscape::
DCH(int n)
{
  if (n < 0)
    return CSI_s "P";
  else
    return CSI_s + CStrUtil::toString(n) + "P";
}

string
CEscape::
SU(int n)
{
  if (n < 0)
    return CSI_s "S";
  else
    return CSI_s + CStrUtil::toString(n) + "S";
}

string
CEscape::
SD(int n)
{
  if (n < 0)
    return CSI_s "T";
  else
    return CSI_s + CStrUtil::toString(n) + "T";
}

string
CEscape::
ECH(int n)
{
  if (n < 0)
    return CSI_s "X";
  else
    return CSI_s + CStrUtil::toString(n) + "X";
}

string
CEscape::
CBT(int n)
{
  if (n < 0)
    return CSI_s "Z";
  else
    return CSI_s + CStrUtil::toString(n) + "Z";
}

string
CEscape::
HPA(int n)
{
  if (n < 0)
    return CSI_s "`";
  else
    return CSI_s + CStrUtil::toString(n) + "`";
}

string
CEscape::
REP(int n)
{
  if (n < 0)
    return CSI_s "b";
  else
    return CSI_s + CStrUtil::toString(n) + "b";
}

string
CEscape::
DA1(int n)
{
  if (n < 0)
    return CSI_s "c";
  else
    return CSI_s + CStrUtil::toString(n) + "c";
}

string
CEscape::
DA2(int n)
{
  if (n < 0)
    return CSI_s ">c";
  else
    return CSI_s + string(">") + CStrUtil::toString(n) + "c";
}

string
CEscape::
VPA(int n)
{
  if (n < 0)
    return CSI_s "d";
  else
    return CSI_s + CStrUtil::toString(n) + "d";
}

string
CEscape::
HVP(int row, int col)
{
  if (row == 1 && col == 1)
    return CSI_s "f";
  else
    return CSI_s + CStrUtil::toString(row) + ";" +
                   CStrUtil::toString(col) + "f";
}

string
CEscape::
TBC(int n)
{
  if (n < 0)
    return CSI_s "g";
  else
    return CSI_s + CStrUtil::toString(n) + "g";
}

string
CEscape::
SM(int n)
{
  if (n < 0)
    return CSI_s "h";
  else
    return CSI_s + CStrUtil::toString(n) + "h";
}

string
CEscape::
DECSET(int n)
{
  if (n < 0)
    return CSI_s "?h";
  else
    return CSI_s "?" + CStrUtil::toString(n) + "h";
}

string
CEscape::
DECSET(int n1, int n2)
{
  return CSI_s "?" + CStrUtil::toString(n1) + ";" + CStrUtil::toString(n2) + "h";
}

string
CEscape::
MC(int n)
{
  if (n < 0)
    return CSI_s "i";
  else
    return CSI_s + CStrUtil::toString(n) + "i";
}

string
CEscape::
DECMC(int n)
{
  if (n < 0)
    return CSI_s "?i";
  else
    return CSI_s "?" + CStrUtil::toString(n) + "i";
}

string
CEscape::
RM(int n)
{
  if (n < 0)
    return CSI_s "l";
  else
    return CSI_s + CStrUtil::toString(n) + "l";
}

string
CEscape::
DECRST(int n)
{
  if (n < 0)
    return CSI_s "?l";
  else
    return CSI_s "?" + CStrUtil::toString(n) + "l";
}

string
CEscape::
DECRST(int n1, int n2)
{
  return CSI_s "?" + CStrUtil::toString(n1) + ";" + CStrUtil::toString(n2) + "l";
}

string
CEscape::
SGR(int n)
{
  if (n < 0)
    return CSI_s "m";
  else
    return CSI_s + CStrUtil::toString(n) + "m";
}

string
CEscape::
SGR(int n, int r, int g, int b)
{
  return CSI_s + CStrUtil::toString(n) + ";" +
                 CStrUtil::toString(r) + ";" +
                 CStrUtil::toString(g) + ";" +
                 CStrUtil::toString(b) + "m";
}

string
CEscape::
DSR(int n)
{
  if (n < 0)
    n = 0;

  return CSI_s + CStrUtil::toString(n) + "n";
}

string
CEscape::
DECDSR(int n)
{
  if (n < 0)
    n = 0;

  return CSI_s "?" + CStrUtil::toString(n) + "n";
}

string
CEscape::
DECSTR()
{
  return CSI_s "!p";
}

string
CEscape::
DECSCL(int n, int m)
{
  if (n < 0)
    n = 0;

  return CSI_s + CStrUtil::toString(n) + ";" +
                 CStrUtil::toString(m) + "\"p";
}

string
CEscape::
DECSTBM(int top, int bottom)
{
  if (top < 0 || bottom < 0)
    return CSI_s "r";
  else
    return CSI_s + CStrUtil::toString(top) + ";" +
                   CStrUtil::toString(bottom) + "r";
}

string
CEscape::
DECCARA(int top, int left, int bottom, int right, int attr)
{
  if (top    < 0) top    = 0;
  if (left   < 0) left   = 0;
  if (bottom < 0) bottom = 0;
  if (right  < 0) right  = 0;
  if (attr   < 0) attr   = 0;

  return CSI_s + CStrUtil::toString(top   ) + ";" +
                 CStrUtil::toString(left  ) + ";";
                 CStrUtil::toString(bottom) + ";";
                 CStrUtil::toString(right ) + ";";
                 CStrUtil::toString(attr  ) + "$r";
}

string
CEscape::
SC()
{
  return CSI_s "s";
}

string
CEscape::
DECRARA(int top, int left, int bottom, int right, int attr)
{
  if (top    < 0) top    = 0;
  if (left   < 0) left   = 0;
  if (bottom < 0) bottom = 0;
  if (right  < 0) right  = 0;
  if (attr   < 0) attr   = 0;

  return CSI_s + CStrUtil::toString(top   ) + ";" +
                 CStrUtil::toString(left  ) + ";";
                 CStrUtil::toString(bottom) + ";";
                 CStrUtil::toString(right ) + ";";
                 CStrUtil::toString(attr  ) + "$t";
}

string
CEscape::
SC1()
{
  return CSI_s "u";
}

string
CEscape::
DECCRA(int src_top, int src_left, int src_bottom, int src_right, int src,
       int dst_top, int dst_left, int dst)
{
  if (src_top    < 0) src_top    = 0;
  if (src_left   < 0) src_left   = 0;
  if (src_bottom < 0) src_bottom = 0;
  if (src_right  < 0) src_right  = 0;
  if (src        < 0) src        = 0;
  if (dst_top    < 0) src_top    = 0;
  if (dst_left   < 0) src_left   = 0;
  if (dst        < 0) dst        = 0;

  return CSI_s + CStrUtil::toString(src_top   ) + ";" +
                 CStrUtil::toString(src_left  ) + ";";
                 CStrUtil::toString(src_bottom) + ";";
                 CStrUtil::toString(src_right ) + ";";
                 CStrUtil::toString(src       ) + ";";
                 CStrUtil::toString(dst_top   ) + ";";
                 CStrUtil::toString(dst_left  ) + ";";
                 CStrUtil::toString(dst       ) + "$v";
}

string
CEscape::
DECEFR(int top, int left, int bottom, int right)
{
  if (top    < 0) top    = 0;
  if (left   < 0) left   = 0;
  if (bottom < 0) bottom = 0;
  if (right  < 0) right  = 0;

  return CSI_s + CStrUtil::toString(top   ) + ";" +
                 CStrUtil::toString(left  ) + ";";
                 CStrUtil::toString(bottom) + ";";
                 CStrUtil::toString(right ) + "'w";
}

string
CEscape::
DECREQTPARM(int n)
{
  if (n < 0) n = 0;

  return CSI_s + CStrUtil::toString(n) + "x";
}

string
CEscape::
DECSACE(int n)
{
  if (n < 0) n = 0;

  return CSI_s + CStrUtil::toString(n) + "*x";
}

string
CEscape::
DECFRA(int c, int top, int left, int bottom, int right)
{
  if (c      < 0) c      = 32;
  if (top    < 0) top    = 0;
  if (left   < 0) left   = 0;
  if (bottom < 0) bottom = 0;
  if (right  < 0) right  = 0;

  return CSI_s + CStrUtil::toString(c     ) + ";" +
                 CStrUtil::toString(top   ) + ";";
                 CStrUtil::toString(left  ) + ";";
                 CStrUtil::toString(bottom) + ";";
                 CStrUtil::toString(right ) + "$x";
}

string
CEscape::
DECELR(int n1, int n2)
{
  if (n1 < 0) n1 = 0;
  if (n2 < 0) n2 = 0;

  return CSI_s + CStrUtil::toString(n1 ) + ";" +
                 CStrUtil::toString(n2) + "'z";
}

string
CEscape::
DECERA(int top, int left, int bottom, int right)
{
  if (top    < 0) top    = 0;
  if (left   < 0) left   = 0;
  if (bottom < 0) bottom = 0;
  if (right  < 0) right  = 0;

  return CSI_s + CStrUtil::toString(top   ) + ";";
                 CStrUtil::toString(left  ) + ";";
                 CStrUtil::toString(bottom) + ";";
                 CStrUtil::toString(right ) + "$z";
}

string
CEscape::
DECSLE(int n)
{
  if (n < 0) n = 0;

  return CSI_s + CStrUtil::toString(n) + "'{";
}

string
CEscape::
DECSERA(int top, int left, int bottom, int right)
{
  if (top    < 0) top    = 0;
  if (left   < 0) left   = 0;
  if (bottom < 0) bottom = 0;
  if (right  < 0) right  = 0;

  return CSI_s + CStrUtil::toString(top   ) + ";";
                 CStrUtil::toString(left  ) + ";";
                 CStrUtil::toString(bottom) + ";";
                 CStrUtil::toString(right ) + "${";
}

string
CEscape::
DECRQLP(int n)
{
  if (n < 0) n = 0;

  return CSI_s + CStrUtil::toString(n) + "'|";
}

void
CEscape::
APC(ostream &os, const string &str)
{
  os << APC(str);
}

string
CEscape::
APC(const string &str)
{
  return ESC_s "_" + str + ESC_s "\\";
}

// ESC[ !?>= <n> "`$*& @ABCDEFGHIKMLMPRSTXZ`bcdfghilmnpqrstuvwxyz{|
//
void
CEscape::
CSI(ostream &os, const string &str)
{
  os << CSI(str);
}

string
CEscape::
CSI(const string &str)
{
  return ESC_s "[" + str;
}

//-------------

static bool parseInteger
             (const vector<string> &words, int pos, int *i, bool opt=false);
static bool checkNumArgs
             (const vector<string> &args, int num, bool opt=false);

string
CEscape::
stringToEscape(const string &str)
{
  int i1, i2, i3, i4, i5;

  vector<string> words;

  CStrUtil::addFields(str, words, ";");

  uint num_words = words.size();

  if      (words[0] == "NUL") return NUL();
  else if (words[0] == "SOH") return SOH();  // Ctrl A
  else if (words[0] == "STX") return STX();  // Ctrl B
  else if (words[0] == "ETX") return ETX();  // Ctrl C
  else if (words[0] == "EOT") return EOT();  // Ctrl D
  else if (words[0] == "ENQ") return ENQ();  // Ctrl E
  else if (words[0] == "ACK") return ACK();  // Ctrl F
  else if (words[0] == "BEL") return BEL();  // Ctrl G
  else if (words[0] == "BS" ) return BS ();  // Ctrl H
  else if (words[0] == "HT" ) return HT ();  // Ctrl I
  else if (words[0] == "TAB") return HT ();  // Ctrl I
  else if (words[0] == "LF" ) return LF ();  // Ctrl J
  else if (words[0] == "NL" ) return LF ();  // Ctrl J
  else if (words[0] == "VT" ) return VT ();  // Ctrl K
  else if (words[0] == "FF" ) return FF ();  // Ctrl L
  else if (words[0] == "NP" ) return FF ();  // Ctrl L
  else if (words[0] == "CR" ) return CR ();  // Ctrl M
  else if (words[0] == "SO" ) return SO ();  // Ctrl N
  else if (words[0] == "SI" ) return SI ();  // Ctrl O
  else if (words[0] == "DLE") return DLE();
  else if (words[0] == "DC1") return DC1();
  else if (words[0] == "DC2") return DC2();
  else if (words[0] == "DC3") return DC3();
  else if (words[0] == "DC4") return DC4();
  else if (words[0] == "NAK") return NAK();
  else if (words[0] == "SYN") return SYN();
  else if (words[0] == "ETB") return ETB();
  else if (words[0] == "CAN") return CAN();
  else if (words[0] == "EM" ) return EM();
  else if (words[0] == "SUB") return SUB();
  else if (words[0] == "DEL") return DEL();

  else if (words[0] == "SP" ) return SP();

  else if (words[0] == "IND") return IND();
  else if (words[0] == "NEL") return NEL();
  else if (words[0] == "HTS") return HTS();
  else if (words[0] == "RI" ) return RI();
  else if (words[0] == "SS2") return SS2();
  else if (words[0] == "SS3") return SS3();
  else if (words[0] == "DCS") return DCS();
  else if (words[0] == "SPA") return SPA();
  else if (words[0] == "EPA") return EPA();
  else if (words[0] == "SOS") return SOS();
  else if (words[0] == "DECID") return DECID();

  else if (words[0] == "WIN") return stringWinOpToEscape(words);
  else if (words[0] == "CSI") return stringCSIOpToEscape(words);
  else if (words[0] == "OSC") return stringOSCOpToEscape(words);

  else if (words[0] == "DECSC" ) return DECSC();
  else if (words[0] == "DECRC" ) return DECRC();
  else if (words[0] == "DECPAM") return DECPAM();
  else if (words[0] == "DECPNM") return DECPNM();

  else if (words[0] == "SCS") return stringSCSOpToEscape(words);

  else if (words[0] == "RIS") return RIS();

  else if (words[0] == "DECALN") return DECALN();

  else if (words[0] == "ICH") {
    if (parseInteger(words, 1, &i1, true))
      return ICH(i1);
    else
      return ICH();
  }
  else if (words[0] == "CUU") {
    if (parseInteger(words, 1, &i1, true))
      return CUU(i1);
    else
      return CUU();
  }
  else if (words[0] == "CUD") {
    if (parseInteger(words, 1, &i1, true))
      return CUD(i1);
    else
      return CUD();
  }
  else if (words[0] == "CUF") {
    if (parseInteger(words, 1, &i1, true))
      return CUF(i1);
    else
      return CUF();
  }
  else if (words[0] == "CUB") {
    if (parseInteger(words, 1, &i1, true))
      return CUB(i1);
    else
      return CUB();
  }
  else if (words[0] == "CNL") {
    if (parseInteger(words, 1, &i1, true))
      return CNL(i1);
    else
      return CNL();
  }
  else if (words[0] == "CPL") {
    if (parseInteger(words, 1, &i1, true))
      return CPL(i1);
    else
      return CPL();
  }
  else if (words[0] == "CHA") {
    if (parseInteger(words, 1, &i1, true))
      return CHA(i1);
    else
      return CHA();
  }
  else if (words[0] == "CUP") {
    if (parseInteger(words, 1, &i1, true) &&
        parseInteger(words, 2, &i2, true))
      return CUP(i1, i2);
    else
      return CUP();
  }
  else if (words[0] == "CHT") {
    if (parseInteger(words, 1, &i1, true))
      return CHT(i1);
    else
      return CHT();
  }
  else if (words[0] == "ED") {
    if (parseInteger(words, 1, &i1, true))
      return ED(i1);
    else
      return ED();
  }
  else if (words[0] == "DECSED") {
    if (parseInteger(words, 1, &i1, true))
      return DECSED(i1);
    else
      return DECSED();
  }
  else if (words[0] == "EL") {
    if (parseInteger(words, 1, &i1, true))
      return EL(i1);
    else
      return EL();
  }
  else if (words[0] == "DECSEL") {
    if (parseInteger(words, 1, &i1, true))
      return DECSEL(i1);
    else
      return DECSEL();
  }
  else if (words[0] == "IL") {
    if (parseInteger(words, 1, &i1, true))
      return IL(i1);
    else
      return IL();
  }
  else if (words[0] == "DL") {
    if (parseInteger(words, 1, &i1, true))
      return DL(i1);
    else
      return DL();
  }
  else if (words[0] == "DCH") {
    if (parseInteger(words, 1, &i1, true))
      return DCH(i1);
    else
      return DCH();
  }
  else if (words[0] == "SU") {
    if (parseInteger(words, 1, &i1, true))
      return SU(i1);
    else
      return SU();
  }
  else if (words[0] == "SD") {
    if (parseInteger(words, 1, &i1, true))
      return SD(i1);
    else
      return SD();
  }
  else if (words[0] == "ECH") {
    if (parseInteger(words, 1, &i1, true))
      return ECH(i1);
    else
      return ECH();
  }
  else if (words[0] == "CBT") {
    if (parseInteger(words, 1, &i1, true))
      return CBT(i1);
    else
      return CBT();
  }
  else if (words[0] == "HPA") {
    if (parseInteger(words, 1, &i1, true))
      return HPA(i1);
    else
      return HPA();
  }
  else if (words[0] == "REP") {
    if (parseInteger(words, 1, &i1, true))
      return REP(i1);
    else
      return REP();
  }
  else if (words[0] == "DA1") {
    if (parseInteger(words, 1, &i1, true))
      return DA1(i1);
    else
      return DA1();
  }
  else if (words[0] == "DA2") {
    if (parseInteger(words, 1, &i1, true))
      return DA2(i1);
    else
      return DA2();
  }
  else if (words[0] == "VPA") {
    if (parseInteger(words, 1, &i1, true))
      return VPA(i1);
    else
      return VPA();
  }
  else if (words[0] == "HVP") {
    if (parseInteger(words, 1, &i1, true) &&
        parseInteger(words, 2, &i2, true))
      return HVP(i1, i2);
    else
      return HVP();
  }
  else if (words[0] == "TBC") {
    if (parseInteger(words, 1, &i1, true))
      return TBC(i1);
    else
      return TBC();
  }
  else if (words[0] == "SM") {
    if (parseInteger(words, 1, &i1, true))
      return SM(i1);
    else
      return SM();
  }
  else if (words[0] == "DECSET") {
    if (parseInteger(words, 1, &i1, true))
      return DECSET(i1);
    else
      return DECSET();
  }
  else if (words[0] == "MC") {
    if (parseInteger(words, 1, &i1, true))
      return MC(i1);
    else
      return MC();
  }
  else if (words[0] == "DECMC") {
    if (parseInteger(words, 1, &i1, true))
      return DECMC(i1);
    else
      return DECMC();
  }
  else if (words[0] == "RM") {
    if (parseInteger(words, 1, &i1, true))
      return RM(i1);
    else
      return RM();
  }
  else if (words[0] == "DECRST") {
    if (parseInteger(words, 1, &i1, true))
      return DECRST(i1);
    else
      return DECRST();
  }
  else if (words[0] == "SGR") {
    if (parseInteger(words, 1, &i1, true)) {
      if (i1 == 38 || i1 == 48) {
        int r, g, b;

        if (parseInteger(words, 2, &r, true) &&
            parseInteger(words, 3, &g, true) &&
            parseInteger(words, 4, &b, true))
          return SGR(i1,r,g,b);

        return SGR(i1);
      }
      else
        return SGR(i1);
    }
    else
      return SGR();
  }
  else if (words[0] == "DSR") {
    if (! parseInteger(words, 1, &i1, true))
      i1 = 0;

    return DSR(i1);
  }
  else if (words[0] == "DECDSR") {
    if (! parseInteger(words, 1, &i1, true))
      i1 = 0;

    return DECDSR(i1);
  }
  else if (words[0] == "DECSTR") {
    return DECSTR();
  }
  else if (words[0] == "DECSCL") {
    if (parseInteger(words, 1, &i1, true) &&
        parseInteger(words, 2, &i2, true))
      return DECSCL(i1, i2);
    else
      return DECSCL();
  }
  else if (words[0] == "DECSTBM") {
    if (parseInteger(words, 1, &i1, true) &&
        parseInteger(words, 2, &i2, true))
      return DECSTBM(i1, i2);
    else
      return DECSTBM();
  }
  else if (words[0] == "DECCARA") {
    if (parseInteger(words, 1, &i1, true) &&
        parseInteger(words, 2, &i2, true) &&
        parseInteger(words, 3, &i3, true) &&
        parseInteger(words, 4, &i4, true) &&
        parseInteger(words, 5, &i5, true))
      return DECCARA(i1, i2, i3, i4, i5);
    else
      return DECCARA();
  }
  else if (words[0] == "SC") {
    return SC();
  }
  else if (words[0] == "DECRARA") {
    if (parseInteger(words, 1, &i1, true) &&
        parseInteger(words, 2, &i2, true) &&
        parseInteger(words, 3, &i3, true) &&
        parseInteger(words, 4, &i4, true) &&
        parseInteger(words, 5, &i5, true))
      return DECRARA(i1, i2, i3, i4, i5);
    else
      return DECRARA();
  }
  else if (words[0] == "DECREQTPARM") {
    if (parseInteger(words, 1, &i1, true))
      return DECREQTPARM(i1);
    else
      return DECREQTPARM();
  }
  else if (words[0] == "DECELR") {
    if (parseInteger(words, 1, &i1, true) &&
        parseInteger(words, 2, &i2, true))
      return DECELR(i1, i2);
    else
      return DECELR();
  }
  else if (words[0] == "DECSLE") {
    if (parseInteger(words, 1, &i1, true))
      return DECSLE(i1);
    else
      return DECSLE();
  }
  else if (words[0] == "DECRQLP") {
    if (parseInteger(words, 1, &i1, true))
      return DECRQLP(i1);
    else
      return DECRQLP();
  }

  else if (words[0] == "s")
    return words[1];
  else if (words[0] == "srep") {
    int n;

    if (! parseInteger(words, 1, &n, false))
      n = 1;

    string s;

    for (int i = 0; i < n; ++i)
      s +=  words[2];

    return s;
  }
  else if (words[0] == "file") {
    CFile file(words[1]);

    string text;

    string line;

    while (file.readLine(line))
      text += line + "\n";

    return text;
  }
  else if (words[0] == "paste") {
    if (num_words > 1)
      return APC("<paste text=\"" + words[1] + "\"/>");
    else
      return APC("<paste/>");
  }
  else if (words[0] == "dir") {
    if (num_words > 1)
      return APC("<state dir=\"" + words[1] + "\"/>");
    else
      return APC("<state dir=\"" + CDir::getCurrent() + "\"/>");
  }
  else if (words[0] == "pixel") {
    if (num_words > 3)
      return APC("<pixel x=\"" + words[1] + "\" y=\"" + words[2] + "\" color=\"" +
                 words[3] + "\"/>");
    else
      return "";
  }
  else if (words[0] == "line") {
    if (num_words > 5)
      return APC("<line x1=\"" + words[1] + "\" y1=\"" + words[2] + "\" "
                 "x2=\"" + words[3] + "\" y2=\"" + words[4] + "\" color=\"" + words[5] + "\"/>");
    else
      return "";
  }
  else {
    cerr << "Invalid command: " <<  words[0] << endl;
    return "";
  }
}

string
CEscape::
stringWinOpToEscape(const vector<string> &words)
{
  if (words.size() < 2) {
    cerr << "Wrong number of arguments: got " <<
            words.size() << " need at least " << 2 << endl;
    return "";
  }

  if      (words[1] == "deiconify") { // 1
    return windowOpDeiconify();
  }
  else if (words[1] == "iconify") { // 2
    return windowOpIconify();
  }
  else if (words[1] == "move") { // 3
    int x, y;

    if (! parseInteger(words, 2, &x) || ! parseInteger(words, 3, &y)) {
      return "";
    }

    return windowOpMove(x, y);
  }
  else if (words[1] == "pixel_resize") { // 4
    int h, w;

    if (! parseInteger(words, 2, &h) || ! parseInteger(words, 3, &w))
      return "";

    return windowOpPixelSize(h, w);
  }
  else if (words[1] == "raise") { // 5
    return windowOpRaise();
  }
  else if (words[1] == "lower") { // 6
    return windowOpLower();
  }
  else if (words[1] == "refresh") { // 7
    return windowOpRefresh();
  }
  else if (words[1] == "resize" || words[1] == "char_resize") { // 8
    int h, w;

    if (! parseInteger(words, 2, &h) || ! parseInteger(words, 3, &w))
      return "";

    return windowOpCharSize(h, w);
  }
  else if (words[1] == "restore_maximized") { // 9;0
    return windowOpRestoreMaximized();
  }
  else if (words[1] == "maximize") { // 9;1
    return windowOpMaximize();
  }
  else if (words[1] == "report_state") { // 11
    return windowOpReportState();
  }
  else if (words[1] == "report_pos") { // 13
    return windowOpReportPos();
  }
  else if (words[1] == "report_pixel_size") { // 14
    return windowOpReportPixelSize();
  }
  else if (words[1] == "report_size" || words[1] == "report_char_size") { // 18
    return windowOpReportCharSize();
  }
  else if (words[1] == "report_screen_size") { // 19
    return windowOpReportScreenSize();
  }
  else if (words[1] == "report_icon_label") { // 20
    return windowOpReportIconLabel();
  }
  else if (words[1] == "report_window_title") { // 21
    return windowOpReportWindowTitle();
  }
  else if (words[1] == "resize_lines") { // 24
    int n;

    if (! parseInteger(words, 2, &n))
      return "";

    if (n < 24) {
      cerr << "Invalid number of lines: " <<  n << endl;
      return "";
    }

    return windowOpResizeNLines(n);
  }
  else {
    cerr << "Invalid WIN op: " <<  words[1] << endl;
    return "";
  }
}

string
CEscape::
stringCSIOpToEscape(const vector<string> &words)
{
  uint num_words = words.size();

  if (num_words == 1)
    return CSI_s;

  string str = CSI_s;

  for (uint i = 1; i < num_words; ++i)
    str += words[i];

  return str;
}

string
CEscape::
stringOSCOpToEscape(const vector<string> &words)
{
  if (! checkNumArgs(words, 2)) return "";

  if      (words[1] == "icon_window_title") { // 0
    if (! checkNumArgs(words, 3)) return "";

    return oscIconWindowTitle(words[2]);
  }
  else if (words[1] == "icon_title") { // 1
    if (! checkNumArgs(words, 3)) return "";

    return oscIconTitle(words[2]);
  }
  else if (words[1] == "window_title") { // 2
    if (! checkNumArgs(words, 3)) return "";

    return oscWindowTitle(words[2]);
  }
  else if (words[1] == "window_prop") { // 3
    if (! checkNumArgs(words, 3)) return "";

    return oscWindowProp(words[2]);
  }
  else if (words[1] == "color") { // 4
    int n;

    if (! parseInteger(words, 2, &n)) return "";

    if (! checkNumArgs(words, 4)) return "";

    return oscColor(n, words[3]);
  }
  else if (words[1] == "fg") { // 10
    if (! checkNumArgs(words, 3)) return "";

    return oscFg(words[2]);
  }
  else if (words[1] == "bg") { // 11
    if (! checkNumArgs(words, 3)) return "";

    return oscBg(words[2]);
  }
  else if (words[1] == "cursor_color") {  // 12
    if (! checkNumArgs(words, 3)) return "";

    return oscCursorColor(words[2]);
  }
  else if (words[1] == "font") { // 50
    if (! checkNumArgs(words, 3)) return "";

    return oscFont(words[2]);
  }
  else {
    cerr << "Invalid OSC op: " <<  words[1] << endl;
    return "";
  }
}

string
CEscape::
stringSCSOpToEscape(const vector<string> &words)
{
  if (! checkNumArgs(words, 3, 3)) return "";

  string str;

  if      (words[1] == "0") {
    str += "(";
    str += words[2][0];
    str += ")B";
    str += SI();
  }
  else if (words[1] == "1") {
    str += ")";
    str += words[2][0];
    str += "(B";
    str += SO();
  }

  return str;
}

string
CEscape::
imageToEscape(const string &filename, int size, int x1, int y1, int x2, int y2)
{
  string str;

  str  = "<image ";
  str += "filename=\"" + filename + "\" ";
  str += "size=\"" + CStrUtil::toString(size) + "\" ";
  str += "x1=\"" + CStrUtil::toString(x1) + "\" ";
  str += "y1=\"" + CStrUtil::toString(y1) + "\" ";
  str += "x2=\"" + CStrUtil::toString(x2) + "\" ";
  str += "y2=\"" + CStrUtil::toString(y2) + "\"";
  str += "/>";

  return CEscape::APC(str);
}

string
CEscape::
imageToEscape(const string &filename, int size)
{
  string str;

  str  = "<image ";
  str += "filename=\"" + filename + "\" ";
  str += "size=\"" + CStrUtil::toString(size) + "\"";
  str += "/>";

  return CEscape::APC(str);
}

string
CEscape::
pixelPointToEscape(int x, int y, const string &color)
{
  string str;

  str  = "<pixel ";
  str += "x=\"" + CStrUtil::toString(x) + "\" ";
  str += "y=\"" + CStrUtil::toString(y) + "\" ";
  str += "color=\"" + color + "\"";
  str += "/>";

  return CEscape::APC(str);
}

string
CEscape::
pixelLineToEscape(int x1, int y1, int x2, int y2, const string &color)
{
  string str;

  str  = "<line ";
  str += "x1=\"" + CStrUtil::toString(x1) + "\" ";
  str += "y1=\"" + CStrUtil::toString(y1) + "\" ";
  str += "x2=\"" + CStrUtil::toString(x2) + "\" ";
  str += "y2=\"" + CStrUtil::toString(y2) + "\" ";
  str += "color=\"" + color + "\"";
  str += "/>";

  return CEscape::APC(str);
}

string
CEscape::
linkToEscape(const string &name, const string &path, const string &type)
{
  string str;

  str  = "<link ";
  str += "name=\"" + name + "\" ";
  str += "dir=\""  + path + "\" ";
  str += "type=\"" + type + "\" ";
  str += "/>";

  return CEscape::APC(str);
}

string
CEscape::
commandToEscape(const string &name, const string &dir, bool start)
{
  string str;

  str  = "<command ";
  str += "name=\"" + name + "\" ";
  str += "dir=\""  + dir + "\" ";
  str += "start=\""  + CStrUtil::toString(start) + "\" ";
  str += "/>";

  return CEscape::APC(str);
}

bool
CEscape::
getWindowCharSize(int *rows, int *cols)
{
  // get window char size

  COSRead::write(0, CEscape::windowOpReportCharSize().c_str());

  std::string result = readResult();

  std::vector<std::string> args;

  if (parseEscape(result, args)) {
    uint num_args = args.size();

    if (num_args == 3 && CStrUtil::isInteger(args[0]) && CStrUtil::isInteger(args[1])) {
      *rows = CStrUtil::toInteger(args[1]);
      *cols = CStrUtil::toInteger(args[2]);

      return true;
    }
  }

  return false;
}

bool
CEscape::
getWindowPixelSize(int *width, int *height)
{
  // get window pixel size

  COSRead::write(0, CEscape::windowOpReportPixelSize().c_str());

  std::string result = readResult();

  std::vector<std::string> args;

  if (parseEscape(result, args)) {
    uint num_args = args.size();

    if (num_args == 3 && CStrUtil::isInteger(args[0]) && CStrUtil::isInteger(args[1])) {
      *width  = CStrUtil::toInteger(args[2]);
      *height = CStrUtil::toInteger(args[1]);

      return true;
    }
  }

  return false;
}

std::string
CEscape::
readResult()
{
  std::string result;

  if (COSRead::wait_read(0, 1, 0))
    COSRead::read(0, result);

  return result;
}

bool
CEscape::
parseEscape(const std::string &str, std::vector<std::string> &args)
{
  int len = str.size();

  if (len < 3) return false;

  if (str[0] != '' || str[1] != '[' || ! isalpha(str[len - 1]))
    return false;

  int i = 2;

  while (i < len && ! isalpha(str[i])) {
    std::string arg;

    while (i < len && str[i] != ';' && ! isalpha(str[i]))
      arg += str[i++];

    if (i < len && str[i] == ';')
      ++i;

    args.push_back(arg);
  }

  return true;
}

bool
CEscape::
parseMouse(const string &str, int *button, int *x, int *y, bool *release)
{
  if (str.size() != 6) return false;

  if (str[0] != '' || str[1] != '[' || str[2] != 'M')
    return false;

  *button  = str[3] - 32;
  *x       = str[4] - 32;
  *y       = str[5] - 32;
  *release = (*button == 3);

  if (*release)
    *button = 1;

  return true;
}

//------------

static bool
parseInteger(const vector<string> &words, int pos, int *i, bool opt)
{
  if (! checkNumArgs(words, pos + 1, opt)) return false;

  if (! CStrUtil::toInteger(words[pos], i)) {
    if (! opt)
      cerr << "Invalid integer: " <<  words[pos] << endl;
    return false;
  }

  return true;
}

static bool
checkNumArgs(const vector<string> &args, int num, bool opt)
{
  if ((int) args.size() < num) {
    if (! opt)
      cerr << "Wrong number of arguments: got " <<
              args.size() << " need at least " << num + 1 << endl;

    return false;
  }

  return true;
}
