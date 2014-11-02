#include <CPathList.h>

#include <CFile.h>
#include <CFileMatch.h>

#include <CEnv.h>
#include <CStrUtil.h>

CPathList::
CPathList() {
}

void
CPathList::
add(const std::string &dir)
{
  dirs_.push_back(dir);
}

void
CPathList::
addEnvValue(const std::string &name)
{
  if (! CEnvInst.exists(name))
    return;

  std::string path = CEnvInst.get(name);

  CStrWords words = CStrUtil::toFields(path, ":");

  int num_words = words.size();

  for (int i = 0; i < num_words; i++)
    dirs_.push_back(words[i].getWord());
}

void
CPathList::
remove(const std::string &dir)
{
  dirs_.remove(dir);
}

void
CPathList::
clear()
{
  dirs_.clear();
}

bool
CPathList::
search(const std::string &file, std::string &path)
{
  std::string file1;

  if (! CFile::expandTilde(file, file1))
    file1 = file;

  if (file1.size() > 0 && file1[0] == '/') {
    CFile file2(file1);

    if (file2.exists() && file2.isRegular() && file2.isExecutable()) {
      path = file2.getPath();

      return true;
    }

    return false;
  }

  CPathDirList::iterator p1 = dirs_.begin();
  CPathDirList::iterator p2 = dirs_.end();

  for ( ; p1 != p2; ++p1) {
    path = *p1 + "/" + file;

    CFile file(path);

    if (file.exists() && file.isRegular() && file.isExecutable())
      return true;
  }

  return false;
}

std::string
CPathList::
mostMatchPrefix(const std::string &prefix)
{
  std::vector<std::string> dirs;
  std::vector<std::string> files;

  if (! matchPrefix(prefix, dirs, files))
    return prefix;

  return CStrUtil::mostMatch(files);
}

std::string
CPathList::
mostMatchPattern(const std::string &pattern)
{
  std::vector<std::string> dirs;
  std::vector<std::string> files;

  if (! matchPattern(pattern, dirs, files))
    return pattern;

  return CStrUtil::mostMatch(files);
}

bool
CPathList::
matchPrefix(const std::string &prefix, std::vector<std::string> &dirs,
            std::vector<std::string> &files)
{
  std::string pattern = prefix + "*";

  return matchPattern(pattern, dirs, files);
}

bool
CPathList::
matchPattern(const std::string &pattern, std::vector<std::string> &dirs,
             std::vector<std::string> &files)
{
  std::string pattern1;

  if (! CFile::expandTilde(pattern, pattern1))
    pattern1 = pattern;

  std::string::size_type pos = pattern1.find('/');

  if (pos != std::string::npos) {
    std::vector<std::string> files1;

    CFileMatch fileMatch;

    fileMatch.matchPattern(pattern1, files1);

    int num_files1 = files1.size();

    for (int j = 0; j < num_files1; j++) {
      std::string fileName;

      if (! CFile::expandTilde(files1[j], fileName))
        fileName = files1[j];

      CFile file(fileName);

      if (file.exists() && file.isRegular() && file.isExecutable()) {
        std::string::size_type pos = files1[j].rfind('/');

        dirs.push_back(file.getDir());

        if (pos != std::string::npos)
          files.push_back(files1[j].substr(pos + 1));
        else
          files.push_back(files1[j]);
      }
    }

    return ! files.empty();
  }

  CPathDirList::iterator p1 = dirs_.begin();
  CPathDirList::iterator p2 = dirs_.end();

  for ( ; p1 != p2; ++p1) {
    std::string full_path = *p1 + "/" + pattern;

    std::vector<std::string> files1;

    CFileMatch fileMatch;

    fileMatch.matchPattern(full_path, files1);

    int num_files1 = files1.size();

    for (int j = 0; j < num_files1; j++) {
      std::string fileName;

      if (! CFile::expandTilde(files1[j], fileName))
        fileName = files1[j];

      CFile file(fileName);

      if (file.exists() && file.isRegular() && file.isExecutable()) {
        std::string::size_type pos = files1[j].rfind('/');

        dirs.push_back(file.getDir());

        if (pos != std::string::npos)
          files.push_back(files1[j].substr(pos + 1));
        else
          files.push_back(files1[j]);
      }
    }
  }

  return ! files.empty();
}

std::string
CPathList::
toEnvValue()
{
  CPathDirList::iterator p1 = dirs_.begin();
  CPathDirList::iterator p2 = dirs_.end();

  std::string str;

  for ( ; p1 != p2; ++p1) {
    if (str.size() > 0)
      str += ":";

    str += *p1;
  }

  return str;
}
