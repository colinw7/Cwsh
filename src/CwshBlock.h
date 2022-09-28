#ifndef CWSH_BLOCK_H
#define CWSH_BLOCK_H

class CwshBlockMgr {
 public:
  CwshBlockMgr(Cwsh *cwsh);
 ~CwshBlockMgr();

  CwshBlock *currentBlock() { return current_block_; }

  CwshBlock *startBlock(CwshBlockType type, const CwshLineArray &lines);
  void       endBlock();

  bool     inBlock () const;
  bool     eof     () const;
  CwshLine readLine() const;

  CwshBlock *find(CwshBlockType type);

  void gotoLabel(const std::string &label);

  bool isBreak      () const { return break_flag_   ; }
  bool isBreakSwitch() const { return breaksw_flag_ ; }
  bool isContinue   () const { return continue_flag_; }
  bool isReturn     () const { return return_flag_  ; }

  int getGotoDepth() const { return goto_depth_; }

  void setBreak      (bool flag) { break_flag_    = flag; }
  void setBreakSwitch(bool flag) { breaksw_flag_  = flag; }
  void setContinue   (bool flag) { continue_flag_ = flag; }
  void setReturn     (bool flag) { return_flag_   = flag; }

 private:
  CPtr<Cwsh>          cwsh_;
  CAutoPtr<CwshBlock> current_block_;
  CwshBlockArray      block_stack_;
  bool                break_flag_    { false };
  bool                breaksw_flag_  { false };
  bool                continue_flag_ { false };
  bool                return_flag_   { false };
  int                 goto_depth_    { 0 };
};

//---

class CwshBlock {
 public:
  CwshBlock(CwshBlockType type, const CwshLineArray &lines);
 ~CwshBlock();

  CwshBlockType getType() const { return type_; }

  const CwshLineArray &getLines() const { return lines_; }

  int getNumLines() const { return int(lines_.size()); }
  const CwshLine &getLine(int i) const { return lines_[uint(i)]; }

  const std::string &getFilename() const { return filename_; }
  void setFilename(const std::string &v) { filename_ = v; }

  int getLineNum() const { return line_num_; }
  void setLineNum(int line_num) { line_num_ = line_num; }

  CwshLine readLine();

  bool eof() const;

  int getLabelLineNum(const std::string &label) const;

 private:
  CwshBlockType type_;
  CwshLineArray lines_;
  std::string   filename_;
  int           line_num_ { 0 };
};

#endif
