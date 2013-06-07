class CGlob;

class CwshWildCard {
 private:
  CAutoPtr<CGlob> glob_;

 public:
  CwshWildCard(const std::string &pattern);
 ~CwshWildCard();

  bool isValid() const;

  bool checkMatch(const std::string &str) const;
};
