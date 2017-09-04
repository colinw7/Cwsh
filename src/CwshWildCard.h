class CGlob;

class CwshWildCard {
 public:
  CwshWildCard(const std::string &pattern);
 ~CwshWildCard();

  bool isValid() const;

  bool checkMatch(const std::string &str) const;

 private:
  CAutoPtr<CGlob> glob_;
};
