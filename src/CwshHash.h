#ifndef CWSH_HASH_H
#define CWSH_HASH_H

namespace Cwsh {

class Hash {
 public:
  Hash(App *cwsh);

  void        addFilePath(const std::string &filename, const std::string &path);
  std::string getFilePath(const std::string &filename);
  void        clearFilePath();
  void        printFilePathStats();
  void        setFilePathActive(bool flag);

 private:
  CPtr<App>       cwsh_;
  HashFilePathMap filePathMap_;
  bool            filePathActive_ { false };
};

}
#endif
