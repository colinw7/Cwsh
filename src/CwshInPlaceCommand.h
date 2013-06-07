#ifndef CWSH_IN_PLACE_COMMAND_H
#define CWSH_IN_PLACE_COMMAND_H

class CwshInPlaceCommand {
 public:
  CwshInPlaceCommand(Cwsh *cwsh, const CwshWord &word);

  bool expand(CwshWordArray &word_array);

 private:
  bool        expandQuotedWord(const CwshWord &word, std::string &word1);
  std::string expandCommand(const std::string &str);
  std::string executeCommands(const CwshCmdArray &cmds);

 private:
  CPtr<Cwsh>      cwsh_;
  const CwshWord &word_;
};

#endif
