#include <iostream>
#include <string>
#include <utility>

#include "cc/util/check.h"

namespace {

class Decimal {
 public:
  explicit Decimal(std::string rep) : rep_(std::move(rep)) {
    CHECK(rep_.size() == 6);
  }

  bool operator==(const Decimal& other) const {
    return rep_ == other.rep_;
  }

  bool operator!=(const Decimal& other) const {
    return rep_ != other.rep_;
  }

  Decimal& operator++() {
    for (auto digit_it = rep_.rbegin(); digit_it != rep_.rend(); ++digit_it) {
      if (*digit_it < '9') {
        ++(*digit_it);
        return *this;
      }
      *digit_it = '0';
    }
    // Overflow.
    CHECK(false);
  }

  bool MatchesRule() const {
    bool some_double = false;
    for (auto digit_it = rep_.begin(); digit_it != rep_.end() - 1; ++digit_it) {
      if (*digit_it > *(digit_it + 1)) return false;
      if (some_double) continue;

      if (*digit_it == *(digit_it + 1)) {
        if (!(digit_it == rep_.begin() || *digit_it != *(digit_it - 1))) {
          continue;
        }
        some_double = digit_it + 2 == rep_.end() ||
                      *digit_it != *(digit_it + 2);
      }
    }
    return some_double;
  }

 private:
  std::string rep_;
};

}  // namespace

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "USAGE: main MINIMUM MAXIMUM\n";
    return 1;
  }

  Decimal min(argv[1]);
  Decimal max(argv[2]);
  int matches = 0;
  for (; min != max; ++min) {
    matches += static_cast<int>(min.MatchesRule());
  }
  std::cout << matches << "\n";
  return 0;
}
