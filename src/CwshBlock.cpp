#include <CwshI.h>

namespace Cwsh {

BlockMgr::
BlockMgr(App *cwsh) :
 cwsh_(cwsh)
{
}

BlockMgr::
~BlockMgr()
{
}

Block *
BlockMgr::
startBlock(BlockType type, const LineArray &lines)
{
  if (inBlock())
    blockStack_.push_back(currentBlock_);

  currentBlock_ = std::make_shared<Block>(type, lines);

  gotoDepth_ = 0;

  return currentBlock_.get();
}

void
BlockMgr::
endBlock()
{
  if (! inBlock())
    CWSH_THROW("Not in block");

  if (! blockStack_.empty()) {
    currentBlock_ = blockStack_.back();

    blockStack_.pop_back();
  }
  else
    currentBlock_ = BlockP();

  if (gotoDepth_ > 1)
    --gotoDepth_;
}

bool
BlockMgr::
inBlock() const
{
  return !!currentBlock_;
}

bool
BlockMgr::
eof() const
{
  if (! inBlock())
    CWSH_THROW("Not in block");

  return currentBlock_->eof();
}

Line
BlockMgr::
readLine() const
{
  if (! inBlock())
    CWSH_THROW("Not in block");

  return currentBlock_->readLine();
}

Block *
BlockMgr::
find(BlockType type)
{
  if (! inBlock())
    return nullptr;

  if (currentBlock_->getType() == type)
    return currentBlock_.get();

  int numBlocks = int(blockStack_.size());

  for (int i = numBlocks - 1; i >= 0; --i) {
    auto *block = blockStack_[i].get();

    if (block->getType() == type)
      return block;
  }

  return nullptr;
}

void
BlockMgr::
gotoLabel(const std::string &label)
{
  if (! inBlock())
    CWSH_THROW("goto: Not in block.");

  gotoDepth_ = 0;

  int lineNum = currentBlock_->getLabelLineNum(label);

  if (lineNum != -1) {
    currentBlock_->setLineNum(lineNum);

    return;
  }

  int numBlocks = int(blockStack_.size());

  for (int i = numBlocks - 1; i >= 0; i--) {
    auto *block = blockStack_[i].get();

    int lineNum1 = block->getLabelLineNum(label);

    if (lineNum1 != -1) {
      block->setLineNum(lineNum1);

      gotoDepth_ = i + 1;

      return;
    }
  }

  CWSH_THROW("goto: Label " + label + "not found.");
}

//----------------

Block::
Block(BlockType type, const LineArray &lines) :
 type_(type), lines_(lines)
{
}

Block::
~Block()
{
}

Line
Block::
readLine()
{
  if (eof())
    CWSH_THROW("Block EOF");

  return lines_[lineNum_++];
}

bool
Block::
eof() const
{
  return (lineNum_ >= int(lines_.size()));
}

int
Block::
getLabelLineNum(const std::string &label) const
{
  int numLines = int(lines_.size());

  for (int i = 0; i < numLines; i++) {
    const auto &line = lines_[i];

    std::vector<std::string> words;

    String::addWords(line.line, words);

    if (words.size() == 2 && words[1] == ":" && words[0] == label)
      return i;
  }

  return -1;
}

}
