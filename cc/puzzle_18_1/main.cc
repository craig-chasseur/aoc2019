#include <array>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "cc/util/check.h"

namespace {

class CaveMap {
 public:
  explicit CaveMap(const char* filename) {
    std::ifstream stream(filename);
    CHECK(stream);
    std::string row;
    while (stream >> row) {
      map_.emplace_back(std::move(row));
    }
    stream.close();

    for (const std::string& row : map_) {
      for (const char cell : row) {
        total_keys_ += std::islower(cell) ? 1 : 0;
      }
    }
  }

  int ShortestKeyPath4Bots() {
    const CellCoords origin = LocateCell('@').value();
    map_[origin.y][origin.x] = '#';
    map_[origin.y][origin.x - 1] = '#';
    map_[origin.y][origin.x + 1] = '#';
    map_[origin.y - 1][origin.x] = '#';
    map_[origin.y + 1][origin.x] = '#';
    std::array<CellCoords, 4> bots{CellCoords{origin.x - 1, origin.y - 1},
                                   CellCoords{origin.x - 1, origin.y + 1},
                                   CellCoords{origin.x + 1, origin.y - 1},
                                   CellCoords{origin.x + 1, origin.y + 1}};
    return ShortestKeyPathImpl(bots, KeySet());
  }

 private:
  struct CellCoords {
    int x = 0;
    int y = 0;

    bool operator==(const CellCoords& other) const {
      return x == other.x && y == other.y;
    }

    template <typename H>
    friend H AbslHashValue(H h, const CellCoords& coords) {
      return H::combine(std::move(h), coords.x, coords.y);
    }

    CellCoords Up() const {
      return CellCoords{x, y - 1};
    }

    CellCoords Down() const {
      return CellCoords{x, y + 1};
    }

    CellCoords Left() const {
      return CellCoords{x - 1, y};
    }

    CellCoords Right() const {
      return CellCoords{x + 1, y};
    }
  };

  class KeySet {
   public:
    KeySet() = default;

    int size() const {
      return __builtin_popcount(rep_);
    }

    bool contains(const char c) const {
      if (c < 'a' || c > 'z') return false;
      return (rep_ & (1 << (c - 'a'))) != 0;
    }

    KeySet With(const char c) const {
      KeySet ret = *this;
      ret.rep_ |= (1 << (c - 'a'));
      return ret;
    }

    bool operator==(const KeySet other) const {
      return rep_ == other.rep_;
    }

    template <typename H>
    friend H AbslHashValue(H h, const KeySet ks) {
      return H::combine(std::move(h), ks.rep_);
    }

   private:
    std::uint32_t rep_ = 0;
  };

  int ShortestKeyPathImpl(const std::array<CellCoords, 4>& bots,
                          const KeySet held_keys) {
    if (held_keys.size() == total_keys_) {
      return 0;
    }

    auto subproblem_key = std::make_pair(bots, held_keys);
    auto solved_it = solved_subproblems_.find(subproblem_key);
    if (solved_it != solved_subproblems_.end()) {
      return solved_it->second;
    }

    int shortest = ShortestKeyPathForBot(bots, held_keys, 0);
    for (int bot_idx = 1; bot_idx < 4; ++bot_idx) {
      shortest = std::min(shortest,
                          ShortestKeyPathForBot(bots, held_keys, bot_idx));
    }
    CHECK(shortest >= 0 && shortest != std::numeric_limits<int>::max());
    solved_subproblems_.try_emplace(subproblem_key, shortest);
    return shortest;
  }

  int ShortestKeyPathForBot(const std::array<CellCoords, 4>& bots,
                            const KeySet held_keys, int bot_idx) {
    int shortest = std::numeric_limits<int>::max();
    absl::flat_hash_set<CellCoords> visited;
    std::vector<CellCoords> current_horizon{bots[bot_idx]};
    std::vector<CellCoords> next_horizon;
    int current_distance = 0;
    while (!current_horizon.empty()) {
      for (const CellCoords cell : current_horizon) {
        const char cell_val = AtCoords(cell);
        if (std::islower(cell_val) && !held_keys.contains(cell_val)) {
          std::array<CellCoords, 4> next_bots = bots;
          next_bots[bot_idx] = cell;
          shortest = std::min(
              shortest,
              current_distance +
                  ShortestKeyPathImpl(next_bots, held_keys.With(cell_val)));
        } else {
          NextReachableFromCell(cell, held_keys, &visited, &next_horizon);
        }
      }
      current_horizon = std::move(next_horizon);
      next_horizon.clear();
      ++current_distance;
    }
    return shortest;
  }

  std::optional<CellCoords> LocateCell(const char target) const {
    for (int y = 0; y < map_.size(); ++y) {
      for (int x = 0; x < map_[y].size(); ++x) {
        if (map_[y][x] == target) return CellCoords{x, y};
      }
    }
    return std::nullopt;
  }

  void NextReachableFromCell(
      const CellCoords current, const KeySet held_keys,
      absl::flat_hash_set<CellCoords>* reachable,
      std::vector<CellCoords>* next) const {
    InsertIfReachableAndNew(current.Up(), held_keys, reachable, next);
    InsertIfReachableAndNew(current.Down(), held_keys, reachable, next);
    InsertIfReachableAndNew(current.Left(), held_keys, reachable, next);
    InsertIfReachableAndNew(current.Right(), held_keys, reachable, next);
  }

  void InsertIfReachableAndNew(
      const CellCoords candidate, const KeySet held_keys,
      absl::flat_hash_set<CellCoords>* reachable,
      std::vector<CellCoords>* next) const {
    if (!Inbounds(candidate)) return;
    const char cell_val = AtCoords(candidate);
    if ((cell_val == '.' || cell_val == '@' || std::islower(cell_val) ||
         held_keys.contains(std::tolower(cell_val))) &&
        reachable->insert(candidate).second) {
      next->push_back(candidate);
    }
  }

  inline bool Inbounds(CellCoords coords) const {
    return coords.y >= 0 && coords.y < map_.size() &&
           coords.x >= 0 && coords.x < map_[coords.y].size();
  }

  inline char AtCoords(CellCoords coords) const {
    return map_[coords.y][coords.x];
  }

  std::vector<std::string> map_;
  int total_keys_ = 0;
  absl::flat_hash_map<std::pair<std::array<CellCoords, 4>, KeySet>, int>
      solved_subproblems_;
};

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  CaveMap cave_map(argv[1]);
  std::cout << cave_map.ShortestKeyPath4Bots() << "\n";
  return 0;
}
