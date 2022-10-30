#include <CBash.h>
#include <CReadLine.h>
#include <CCommand.h>
#include <CStrParse.h>
#include <CFile.h>
#include <CFileMatch.h>
#include <CDir.h>
#include <CGlob.h>
#include <CStrUtil.h>
#include <COSUser.h>

namespace CBash {

App::
App()
{
  readLine_ = new CReadLine;
}

App::
~App()
{
}

void
App::
init(int, char **)
{
  addShellCommands();
}

void
App::
addShellCommands()
{
  shellCommands_["cd"  ] = new CdCommand  (this);
  shellCommands_["exit"] = new ExitCommand(this);
}

void
App::
mainLoop()
{
  while (! eof()) {
    auto line = getLine();

    Cmd cmd;

    parseCommand(line, cmd);

    processCommand(cmd);
  }
}

void
App::
parseCommand(const std::string &line, Cmd &cmd) const
{
  CStrParse parse(line);

  parse.skipSpace();

  auto pos = parse.getPos();

  parse.skipNonSpace();

  cmd.name = parse.getBefore(pos);

  while (! parse.eof()) {
    parse.skipSpace();

    auto pos1 = parse.getPos();

    parse.skipNonSpace();

    auto arg = parse.getAt(pos1);

    std::vector<std::string> args;

    expandArg(arg, args);

    for (const auto &arg1 : args)
      cmd.args.push_back(arg1);
  }
}

void
App::
expandArg(const std::string &arg, std::vector<std::string> &args) const
{
  CGlob glob(arg);

  if (! glob.isPattern()) {
    args.push_back(arg);
    return;
  }

  glob.setAllowOr(false);
  glob.setAllowNonPrintable(true);

  std::vector<std::string> files1;

  CFileMatch fileMatch;

  if (! fileMatch.matchPattern(arg, files1)) {
    args.push_back(arg);
    return;
  }

  CStrUtil::sort(files1);

  for (const auto &file1 : files1)
    args.push_back(file1);
}

bool
App::
eof() const
{
  return false;
}

std::string
App::
getLine() const
{
  auto line = readLine_->readLine();

  return line;
}

void
App::
processCommand(const Cmd &cmd)
{
  auto p = shellCommands_.find(cmd.name);

  if (p != shellCommands_.end())
    return (*p).second->exec(cmd);

  CCommand command(cmd.name, cmd.name, cmd.args);

  command.start();

  command.wait();
}

std::string
App::
lookupDir(const std::string &dirname) const
{
  if (CFile::exists(dirname) && CFile::isDirectory(dirname))
    return dirname;

  return "";
}

bool
App::
changeDir(const std::string &dirname) const
{
  if (! CDir::changeDir(dirname))
    return false;

  return true;
}

//---

void
CdCommand::
exec(const Cmd &cmd)
{
  auto num_args = cmd.args.size();

  if (num_args == 0) {
    auto dirname = COSUser::getUserHome();

    app_->changeDir(dirname);
  }
  else {
    for (size_t i = 0; i < num_args; ++i) {
      auto dirname = CStrUtil::stripSpaces(cmd.args[i]);

      if (dirname == "")
        dirname = COSUser::getUserHome();

      dirname = app_->lookupDir(dirname);

      app_->changeDir(dirname);
    }
  }
}

//---

void
ExitCommand::
exec(const Cmd &)
{
  exit(0);
}

}
