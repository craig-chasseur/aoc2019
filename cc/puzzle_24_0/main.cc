#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>

#include "absl/container/flat_hash_set.h"
#include "cc/util/check.h"

namespace {

class Grid {
 public:
  Grid() = default;

  std::uint32_t Biodiversity() const {
    return rep_;
  }

  void SetBug(int x, int y, bool present) {
    const int pos = y * 5 + x;
    if (present) {
      rep_ |= (1u << pos);
    } else {
      rep_ &= ~(1u << pos);
    }
  }

  Grid next() const {
    Grid next;
    for (int y = 0; y < 5; ++y) {
      for (int x = 0; x < 5; ++x) {
        const int bugs_adjacent = BugsAdjacent(x, y);
        if (At(x, y)) {
          next.SetBug(x, y, bugs_adjacent == 1);
        } else {
          next.SetBug(x, y, bugs_adjacent == 1 || bugs_adjacent == 2);
        }
      }
    }
    return next;
  }

  bool operator==(const Grid other) const {
    return rep_ == other.rep_;
  }

  template <typename H>
  friend H AbslHashValue(H h, const Grid grid) {
    return H::combine(std::move(h), grid.rep_);
  }

 private:
  bool At(int x, int y) const {
    if ((x < 0) || (x > 4) || (y < 0) || (y > 4)) {
      return false;
    }

    int offset = y * 5 + x;
    return (rep_ & (1u << offset)) != 0;
  }

  int BugsAdjacent(int x, int y) const {
    return static_cast<int>(At(x - 1, y)) + static_cast<int>(At(x + 1, y)) +
           static_cast<int>(At(x, y - 1)) + static_cast<int>(At(x, y + 1));
  }

  std::uint32_t rep_ = 0;
};

Grid LoadGrid(const char* filename) {
  Grid grid;

  std::ifstream stream(filename);
  CHECK(stream);
  int y = 0;
  std::string row;
  while (std::getline(stream, row)) {
    if (row.back() == '\n') row.pop_back();
    CHECK(row.size() == 5);
    for (int x = 0; x < row.size(); ++x) {
      grid.SetBug(x, y, row[x] == '#');
    }
    ++y;
  }
  stream.close();
  CHECK(y == 5);
  return grid;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }

  Grid grid = LoadGrid(argv[1]);
  absl::flat_hash_set<Grid> all_grids{grid};
  for (;;) {
    grid = grid.next();
    if (!all_grids.emplace(grid).second) break;
  }
  std::cout << grid.Biodiversity() << "\n";

  return 0;
}
