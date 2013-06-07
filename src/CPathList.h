#ifndef CPathList_H
#define CPathList_H

#include <list>
#include <vector>
#include <string>

class CPathList {
 public:
  CPathList();

  void add(const std::string &dir);
  void addEnvValue(const std::string &dir);
  void remove(const std::string &dir);
  void clear();

  bool search(const std::string &file, std::string &path);

  std::string mostMatchPrefix(const std::string &prefix);
  std::string mostMatchPattern(const std::string &pattern);

  bool matchPrefix(const std::string &prefix, std::vector<std::string> &dirs,
                   std::vector<std::string> &files);
  bool matchPattern(const std::string &pattern, std::vector<std::string> &dirs,
                    std::vector<std::string> &files);

  std::string toEnvValue();

 private:
  typedef std::list<std::string> CPathDirList;

  CPathDirList dirs_;
};

#endif
