#include "CwshI.h"

CwshBlockMgr::
CwshBlockMgr(Cwsh *cwsh) :
 cwsh_(cwsh)
{
  break_flag_    = false;
  breaksw_flag_  = false;
  continue_flag_ = false;
  return_flag_   = false;

  goto_depth_ = 0;
}

CwshBlockMgr::
~CwshBlockMgr()
{
  std::for_each(block_stack_.begin(), block_stack_.end(), CDeletePointer());
}

void
CwshBlockMgr::
startBlock(CwshBlockType type, const CwshLineArray &lines)
{
  if (inBlock()) {
    block_stack_.push_back(current_block_);

    current_block_.release();
  }

  current_block_ = new CwshBlock(type, lines);

  goto_depth_ = 0;
}

void
CwshBlockMgr::
endBlock()
{
  if (! inBlock())
    CWSH_THROW("Not in block");

  if (! block_stack_.empty()) {
    current_block_ = block_stack_.back();

    block_stack_.pop_back();
  }
  else
    current_block_ = NULL;

  if (goto_depth_ > 1)
    goto_depth_--;
}

bool
CwshBlockMgr::
inBlock() const
{
  return (current_block_ != NULL);
}

bool
CwshBlockMgr::
eof() const
{
  if (! inBlock())
    CWSH_THROW("Not in block");

  return current_block_->eof();
}

CwshLine
CwshBlockMgr::
readLine() const
{
  if (! inBlock())
    CWSH_THROW("Not in block");

  return current_block_->readLine();
}

CwshBlock *
CwshBlockMgr::
find(CwshBlockType type)
{
  if (! inBlock())
    return NULL;

  if (current_block_->getType() == type)
    return current_block_;

  int num_blocks = block_stack_.size();

  for (int i = num_blocks - 1; i >= 0; --i) {
    CwshBlock *block = block_stack_[i];

    if (block->getType() == type)
      return block;
  }

  return NULL;
}

void
CwshBlockMgr::
gotoLabel(const string &label)
{
  if (! inBlock())
    CWSH_THROW("goto: Not in block.");

  goto_depth_ = 0;

  int line_num = current_block_->getLabelLineNum(label);

  if (line_num != -1) {
    current_block_->setLineNum(line_num);

    return;
  }

  int num_blocks = block_stack_.size();

  for (int i = num_blocks - 1; i >= 0; i--) {
    CwshBlock *block = block_stack_[i];

    int line_num = block->getLabelLineNum(label);

    if (line_num != -1) {
      block->setLineNum(line_num);

      goto_depth_ = i + 1;

      return;
    }
  }

  CWSH_THROW("goto: Label " + label + "not found.");
}

//----------------

CwshBlock::
CwshBlock(CwshBlockType type, const CwshLineArray &lines) :
 type_(type), lines_(lines)
{
  line_num_ = 0;
}

CwshBlock::
~CwshBlock()
{
}

CwshLine
CwshBlock::
readLine()
{
  if (eof())
    CWSH_THROW("Block EOF");

  return lines_[line_num_++];
}

bool
CwshBlock::
eof() const
{
  return (line_num_ >= (int) lines_.size());
}

int
CwshBlock::
getLabelLineNum(const string &label) const
{
  int num_lines = lines_.size();

  for (int i = 0; i < num_lines; i++) {
    const CwshLine &line = lines_[i];

    vector<string> words;

    CwshString::addWords(line, words);

    if (words.size() == 2 && words[1] == ":" && words[0] == label)
      return i;
  }

  return -1;
}
