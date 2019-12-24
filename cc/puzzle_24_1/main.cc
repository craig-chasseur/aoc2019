#include <cstdint>
#include <deque>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <utility>

#include "absl/container/flat_hash_set.h"
#include "cc/util/check.h"

namespace {

class GridLevel {
 public:
  GridLevel() = default;

  bool empty() const {
    return rep_ == 0;
  }

  void SetBug(int x, int y, bool present) {
    CHECK(!(x == 2 && y == 2));
    const int pos = y * 5 + x;
    if (present) {
      rep_ |= (1u << pos);
    } else {
      rep_ &= ~(1u << pos);
    }
  }

  GridLevel Next(const GridLevel* above, const GridLevel* below) const {
    GridLevel next;
    for (int y = 0; y < 5; ++y) {
      for (int x = 0; x < 5; ++x) {
        if (y == 2 && x == 2) continue;
        const int bugs_adjacent = BugsAdjacent(x, y, above, below);
        if (At(x, y)) {
          next.SetBug(x, y, bugs_adjacent == 1);
        } else {
          next.SetBug(x, y, bugs_adjacent == 1 || bugs_adjacent == 2);
        }
      }
    }
    return next;
  }

  int NumBugs() const {
    return __builtin_popcount(rep_);
  }

 private:
  bool At(int x, int y) const {
    CHECK(!(x == 2 && y == 2));
    if ((x < 0) || (x > 4) || (y < 0) || (y > 4)) {
      return false;
    }

    int offset = y * 5 + x;
    return (rep_ & (1u << offset)) != 0;
  }

  int BugsAdjacent(int x, int y,
                   const GridLevel* above, const GridLevel* below) const {
    int bugs = 0;

    // Left.
    if (x == 0) {
      // Adjacent to an upper-level cell.
      if (above != nullptr) bugs += static_cast<int>(above->At(1, 2));
    } else if (x == 3 && y == 2) {
      // Adjacent to 4 lower-level cells.
      if (below != nullptr) {
        bugs += static_cast<int>(below->At(4, 0)) +
                static_cast<int>(below->At(4, 1)) +
                static_cast<int>(below->At(4, 2)) +
                static_cast<int>(below->At(4, 3)) +
                static_cast<int>(below->At(4, 4));
      }
    } else {
      // Adjacent to a cell on the same level.
      bugs += static_cast<int>(At(x - 1, y));
    }

    // Right.
    if (x == 4) {
      // Adjacent to an upper-level cell.
      if (above != nullptr) bugs += static_cast<int>(above->At(3, 2));
    } else if (x == 1 && y == 2) {
      // Adjacent to 4 lower-level cells.
      if (below != nullptr) {
        bugs += static_cast<int>(below->At(0, 0)) +
                static_cast<int>(below->At(0, 1)) +
                static_cast<int>(below->At(0, 2)) +
                static_cast<int>(below->At(0, 3)) +
                static_cast<int>(below->At(0, 4));
      }
    } else {
      // Adjacent to a cell on the same level.
      bugs += static_cast<int>(At(x + 1, y));
    }

    // Up.
    if (y == 0) {
      // Adjacent to an upper-level cell.
      if (above != nullptr) bugs += static_cast<int>(above->At(2, 1));
    } else if (y == 3 && x == 2) {
      // Adjacent to 4 lower-level cells.
      if (below != nullptr) {
        bugs += static_cast<int>(below->At(0, 4)) +
                static_cast<int>(below->At(1, 4)) +
                static_cast<int>(below->At(2, 4)) +
                static_cast<int>(below->At(3, 4)) +
                static_cast<int>(below->At(4, 4));
      }
    } else {
      // Adjacent to a cell on the same level.
      bugs += static_cast<int>(At(x, y - 1));
    }

    // Down.
    if (y == 4) {
      // Adjacent to an upper-level cell.
      if (above != nullptr) bugs += static_cast<int>(above->At(2, 3));
    } else if (y == 1 && x == 2) {
      // Adjacent to 4 lower-level cells.
      if (below != nullptr) {
        bugs += static_cast<int>(below->At(0, 0)) +
                static_cast<int>(below->At(1, 0)) +
                static_cast<int>(below->At(2, 0)) +
                static_cast<int>(below->At(3, 0)) +
                static_cast<int>(below->At(4, 0));
      }
    } else {
      // Adjacent to a cell on the same level.
      bugs += static_cast<int>(At(x, y + 1));
    }

    return bugs;
  }

  std::uint32_t rep_ = 0;
};

class Grid {
 public:
  explicit Grid(GridLevel initial_level) : levels_{initial_level} {}

  void RunMinutes(const int minutes) {
    for (int i = 0; i < minutes; ++i) {
      if (!levels_.front().empty()) levels_.emplace_front();
      if (!levels_.back().empty()) levels_.emplace_back();

      std::deque<GridLevel> next_levels(levels_.size());
      for (int level_pos = 0; level_pos < next_levels.size(); ++level_pos) {
        next_levels[level_pos] = levels_[level_pos].Next(
            level_pos == 0 ? nullptr : &levels_[level_pos - 1],
            level_pos == levels_.size() - 1 ? nullptr
                                            : &levels_[level_pos + 1]);
      }
      levels_ = std::move(next_levels);
    }
  }

  int TotalBugs() const {
    return std::accumulate(levels_.begin(), levels_.end(), 0,
        [](int sum, const GridLevel level) { return sum + level.NumBugs(); });
  }

 private:
  std::deque<GridLevel> levels_;
};

GridLevel LoadGrid(const char* filename) {
  GridLevel grid;

  std::ifstream stream(filename);
  CHECK(stream);
  int y = 0;
  std::string row;
  while (std::getline(stream, row)) {
    if (row.back() == '\n') row.pop_back();
    CHECK(row.size() == 5);
    for (int x = 0; x < row.size(); ++x) {
      if (y == 2 && x == 2) {
        CHECK(row[x] == '.');
        continue;
      }
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

  Grid grid(LoadGrid(argv[1]));
  grid.RunMinutes(200);
  std::cout << grid.TotalBugs() << "\n";

  return 0;
}
