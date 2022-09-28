#include <CwshI.h>

CwshBlockMgr::
CwshBlockMgr(Cwsh *cwsh) :
 cwsh_(cwsh)
{
}

CwshBlockMgr::
~CwshBlockMgr()
{
  for (auto &block : block_stack_)
    delete block;
}

CwshBlock *
CwshBlockMgr::
startBlock(CwshBlockType type, const CwshLineArray &lines)
{
  if (inBlock()) {
    block_stack_.push_back(current_block_);

    current_block_.release();
  }

  current_block_ = new CwshBlock(type, lines);

  goto_depth_ = 0;

  return current_block_;
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
    current_block_ = nullptr;

  if (goto_depth_ > 1)
    --goto_depth_;
}

bool
CwshBlockMgr::
inBlock() const
{
  return current_block_;
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
    return nullptr;

  if (current_block_->getType() == type)
    return current_block_;

  int num_blocks = int(block_stack_.size());

  for (int i = num_blocks - 1; i >= 0; --i) {
    CwshBlock *block = block_stack_[i];

    if (block->getType() == type)
      return block;
  }

  return nullptr;
}

void
CwshBlockMgr::
gotoLabel(const std::string &label)
{
  if (! inBlock())
    CWSH_THROW("goto: Not in block.");

  goto_depth_ = 0;

  int line_num = current_block_->getLabelLineNum(label);

  if (line_num != -1) {
    current_block_->setLineNum(line_num);

    return;
  }

  int num_blocks = int(block_stack_.size());

  for (int i = num_blocks - 1; i >= 0; i--) {
    CwshBlock *block = block_stack_[i];

    int lineNum1 = block->getLabelLineNum(label);

    if (lineNum1 != -1) {
      block->setLineNum(lineNum1);

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
  return (line_num_ >= int(lines_.size()));
}

int
CwshBlock::
getLabelLineNum(const std::string &label) const
{
  int num_lines = int(lines_.size());

  for (int i = 0; i < num_lines; i++) {
    const CwshLine &line = lines_[i];

    std::vector<std::string> words;

    CwshString::addWords(line.line, words);

    if (words.size() == 2 && words[1] == ":" && words[0] == label)
      return i;
  }

  return -1;
}
