#ifndef CESCAPE_COLORS_H
#define CESCAPE_COLORS_H

#include <CEscapeType.h>
#include <CRGBA.h>
#include <map>

#define CEscapeColorsInst CEscapeColors::instance()

class CEscapeColors {
 public:
  static CEscapeColors *instance();

 ~CEscapeColors();

  const CRGBA &getBg() const { return bg_; }
  void setBg(const CRGBA &bg);

  const CRGBA &getFg() const { return fg_; }
  void setFg(const CRGBA &fg);

  const CRGBA &getColor(CEscapeColor type) const;
  void setColor(CEscapeColor type, const CRGBA &rgba);

  bool isDim(CEscapeColor type) const;
  CEscapeColor setDim(CEscapeColor type) const;

  CEscapeColor encode(const CRGBA &c) const;
  CRGBA decode(const CEscapeColor &c) const;

  std::string colorFgStr(const CRGBA &c, bool rgba=false) const;
  std::string colorBgStr(const CRGBA &c, bool rgba=false) const;

 private:
  CEscapeColors();

  void init();

 private:
  typedef std::map<CEscapeColor,CRGBA> Colors;

  CRGBA  bg_, fg_;
  Colors colors_;
};

#endif
