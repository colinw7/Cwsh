class CGlob;

namespace Cwsh {

class WildCard {
 public:
  WildCard(const std::string &pattern);
 ~WildCard();

  bool isValid() const;

  bool checkMatch(const std::string &str) const;

 private:
  using GlobP = std::shared_ptr<CGlob>;

  GlobP glob_;
};

}
