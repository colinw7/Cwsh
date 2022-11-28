#ifndef CWSH_IN_PLACE_COMMAND_H
#define CWSH_IN_PLACE_COMMAND_H

namespace Cwsh {

class InPlaceCommand {
 public:
  InPlaceCommand(App *cwsh, const Word &word);

  bool expand(WordArray &wordArray);

 private:
  bool        expandQuotedWord(const Word &word, std::string &word1);
  std::string expandCommand(const std::string &str);
  std::string executeCommands(const CmdArray &cmds);

 private:
  CPtr<App>   cwsh_;
  const Word &word_;
};

}

#endif
