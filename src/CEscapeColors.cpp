#include <CEscapeColors.h>
#include <CRGBUtil.h>

CEscapeColors *
CEscapeColors::
instance()
{
  static CEscapeColors *inst;

  if (! inst)
    inst = new CEscapeColors;

  return inst;
}

CEscapeColors::
CEscapeColors()
{
  init();
}

CEscapeColors::
~CEscapeColors()
{
}

void
CEscapeColors::
init()
{
  bg_ = CRGBA(0,0,0);
  fg_ = CRGBA(1,1,1);

  colors_[CEscapeColor::BLACK  ] = CRGBA(0.0,0.0,0.0);
  colors_[CEscapeColor::RED    ] = CRGBA(1.0,0.0,0.0);
  colors_[CEscapeColor::GREEN  ] = CRGBA(0.0,1.0,0.0);
  colors_[CEscapeColor::YELLOW ] = CRGBA(1.0,1.0,0.0);
  colors_[CEscapeColor::BLUE   ] = CRGBA(0.0,0.0,1.0);
  colors_[CEscapeColor::MAGENTA] = CRGBA(1.0,0.0,1.0);
  colors_[CEscapeColor::CYAN   ] = CRGBA(0.0,1.0,1.0);
  colors_[CEscapeColor::WHITE  ] = CRGBA(1.0,1.0,1.0);

  colors_[CEscapeColor::ALT_BLACK  ] = CRGBA(0.2,0.2,0.2);
  colors_[CEscapeColor::ALT_WHITE  ] = CRGBA(0.8,0.8,0.8);
  colors_[CEscapeColor::ALT_RED    ] = CRGBUtil::HSVtoRGB(CHSV(  0.0,1.0,0.5));
  colors_[CEscapeColor::ALT_GREEN  ] = CRGBUtil::HSVtoRGB(CHSV(120.0,1.0,0.5));
  colors_[CEscapeColor::ALT_YELLOW ] = CRGBUtil::HSVtoRGB(CHSV( 60.0,1.0,0.5));
  colors_[CEscapeColor::ALT_BLUE   ] = CRGBUtil::HSVtoRGB(CHSV(240.0,1.0,0.5));
  colors_[CEscapeColor::ALT_MAGENTA] = CRGBUtil::HSVtoRGB(CHSV(300.0,1.0,0.5));
  colors_[CEscapeColor::ALT_CYAN   ] = CRGBUtil::HSVtoRGB(CHSV(180.0,1.0,0.5));
}

void
CEscapeColors::
setBg(const CRGBA &bg)
{
  bg_ = bg;
}

void
CEscapeColors::
setFg(const CRGBA &fg)
{
  fg_ = fg;
}

const CRGBA &
CEscapeColors::
getColor(CEscapeColor type) const
{
  static CRGBA c;

  bool dim = isDim(type);

  CEscapeColor type1 = (dim ? CEscapeColor(uint(type) & ~uint(CEscapeColor::DIM)) : type);

  if      (type1 == CEscapeColor::BG)
    c = getBg();
  else if (type1 == CEscapeColor::FG)
    c = getFg();
  else {
    auto p = colors_.find(type1);

    if (p != colors_.end())
      c = (*p).second;
  }

  if (dim)
    c *= 0.8;

  return c;
}

void
CEscapeColors::
setColor(CEscapeColor type, const CRGBA &rgba)
{
  bool dim = isDim(type);

  CEscapeColor type1 = (dim ? CEscapeColor(uint(type) & ~uint(CEscapeColor::DIM)) : type);

  if (type1 == CEscapeColor::BG) { setBg(rgba); return; }
  if (type1 == CEscapeColor::FG) { setFg(rgba); return; }

  colors_[type1] = rgba;
}

bool
CEscapeColors::
isDim(CEscapeColor type) const
{
  return (uint(type) & uint(CEscapeColor::DIM));
}

CEscapeColor
CEscapeColors::
setDim(CEscapeColor type) const
{
  return CEscapeColor(uint(type) | uint(CEscapeColor::DIM));
}

CEscapeColor
CEscapeColors::
encode(const CRGBA &c) const
{
  return CEscapeColor(c.encodeARGB());
}

CRGBA
CEscapeColors::
decode(const CEscapeColor &c) const
{
  if (uint(c) < 64)
    return getColor(c);

  //---

  CRGBA rgba;

  CRGBA::decodeARGB(uint(c), rgba);

  return rgba;
}

std::string
CEscapeColors::
colorFgStr(const CRGBA &c, bool rgb) const
{
  if (rgb) {
    return "[38;" + std::to_string(c.getRedI  ()) + ";" +
                      std::to_string(c.getGreenI()) + ";" +
                      std::to_string(c.getRedI  ()) + "m";
  }
  else {
    int ind = (c.getBlue () >= 0.5 ? 4 : 0) |
              (c.getGreen() >= 0.5 ? 2 : 0) |
              (c.getRed  () >= 0.5 ? 1 : 0);

    return "[" + std::to_string(30 + ind) + "m";
  }
}

std::string
CEscapeColors::
colorBgStr(const CRGBA &c, bool rgb) const
{
  if (rgb) {
    return "[48;" + std::to_string(c.getRedI  ()) + ";" +
                      std::to_string(c.getGreenI()) + ";" +
                      std::to_string(c.getRedI  ()) + "m";
  }
  else {
    int ind = (c.getBlue () >= 0.5 ? 4 : 0) |
              (c.getGreen() >= 0.5 ? 2 : 0) |
              (c.getRed  () >= 0.5 ? 1 : 0);

    return "[" + std::to_string(40 + ind) + "m";
  }
}
