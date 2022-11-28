#ifndef CWSH_BLOCK_H
#define CWSH_BLOCK_H

namespace Cwsh {

//---

class BlockMgr {
 public:
  BlockMgr(App *cwsh);
 ~BlockMgr();

  Block *currentBlock() { return currentBlock_.get(); }

  Block *startBlock(BlockType type, const LineArray &lines);
  void   endBlock();

  bool inBlock () const;
  bool eof     () const;
  Line readLine() const;

  Block *find(BlockType type);

  void gotoLabel(const std::string &label);

  bool isBreak      () const { return breakFlag_   ; }
  bool isBreakSwitch() const { return breakswFlag_ ; }
  bool isContinue   () const { return continueFlag_; }
  bool isReturn     () const { return returnFlag_  ; }

  int getGotoDepth() const { return gotoDepth_; }

  void setBreak      (bool flag) { breakFlag_    = flag; }
  void setBreakSwitch(bool flag) { breakswFlag_  = flag; }
  void setContinue   (bool flag) { continueFlag_ = flag; }
  void setReturn     (bool flag) { returnFlag_   = flag; }

 private:
  CPtr<App>  cwsh_;
  BlockP     currentBlock_;
  BlockArray blockStack_;
  bool       breakFlag_    { false };
  bool       breakswFlag_  { false };
  bool       continueFlag_ { false };
  bool       returnFlag_   { false };
  int        gotoDepth_    { 0 };
};

//---

class Block {
 public:
  Block(BlockType type, const LineArray &lines);
 ~Block();

  BlockType getType() const { return type_; }

  const LineArray &getLines() const { return lines_; }

  int getNumLines() const { return int(lines_.size()); }
  const Line &getLine(int i) const { return lines_[uint(i)]; }

  const std::string &getFilename() const { return filename_; }
  void setFilename(const std::string &v) { filename_ = v; }

  int getLineNum() const { return lineNum_; }
  void setLineNum(int lineNum) { lineNum_ = lineNum; }

  Line readLine();

  bool eof() const;

  int getLabelLineNum(const std::string &label) const;

 private:
  BlockType   type_;
  LineArray   lines_;
  std::string filename_;
  int         lineNum_ { 0 };
};

//---

}

#endif
