#ifndef CWSH_HASH_H
#define CWSH_HASH_H

class CwshHash {
 public:
  CwshHash(Cwsh *cwsh);

  void        addFilePath(const std::string &filename, const std::string &path);
  std::string getFilePath(const std::string &filename);
  void        clearFilePath();
  void        printFilePathStats();
  void        setFilePathActive(bool flag);

 private:
  CPtr<Cwsh>          cwsh_;
  CwshHashFilePathMap file_path_map_;
  bool                file_path_active_;
};

#endif
