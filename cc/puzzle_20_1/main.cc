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

class MazeMap {
 public:
  explicit MazeMap(const char* filename) {
    std::ifstream stream(filename);
    CHECK(stream);
    std::string row;
    while (std::getline(stream, row)) {
      map_.emplace_back(std::move(row));
    }
    stream.close();

    absl::flat_hash_map<std::array<char, 2>, CellCoords> labelled_portals;
    for (int y = 0; y < map_.size(); ++y) {
      for (int x = 0; x < map_[y].size(); ++x) {
        CellCoords coords{x, y};
        if (!std::isupper(AtCoords(coords))) continue;
        auto maybe_label = LabelAt(coords);
        if (!maybe_label.has_value()) continue;
        auto [label, open] = std::move(maybe_label).value();

        if (label[0] == 'A' && label[1] == 'A') {
          start_ = open;
          continue;
        }
        if (label[0] == 'Z' && label[1] == 'Z') {
          finish_ = open;
          continue;
        }

        auto insert_result = labelled_portals.try_emplace(label, open);
        if (insert_result.second) continue;
        const bool open_outer = OuterEdge(open);
        CHECK(portals_.try_emplace(
            insert_result.first->second,
            TaggedPortal{open, !open_outer}).second);
        CHECK(portals_.try_emplace(
            open,
            TaggedPortal{insert_result.first->second, open_outer}).second);
        labelled_portals.erase(insert_result.first);
      }
    }
    CHECK(labelled_portals.empty());
  }

  int ShortestPath() const {
    absl::flat_hash_set<LevelCoords> visited;
    std::vector<LevelCoords> current_horizon{{start_, 0}};
    std::vector<LevelCoords> next_horizon;
    int current_distance = 0;
    while (!current_horizon.empty()) {
      for (const LevelCoords cell : current_horizon) {
        if (cell.coords == finish_ && cell.level == 0) return current_distance;
        NextReachableFromCell(cell, &visited, &next_horizon);
      }
      current_horizon = std::move(next_horizon);
      next_horizon.clear();
      ++current_distance;
    }
    std::cerr << "No path exists\n";
    CHECK(false);
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

  struct TaggedPortal {
    CellCoords coords;
    bool outer = false;
  };

  struct LevelCoords {
    CellCoords coords;
    int level = 0;

    bool operator==(const LevelCoords& other) const {
      return coords == other.coords && level == other.level;
    }

    template <typename H>
    friend H AbslHashValue(H h, const LevelCoords& coords) {
      return H::combine(std::move(h), coords.coords, coords.level);
    }

    LevelCoords Up() const {
      return {coords.Up(), level};
    }

    LevelCoords Down() const {
      return {coords.Down(), level};
    }

    LevelCoords Left() const {
      return {coords.Left(), level};
    }

    LevelCoords Right() const {
      return {coords.Right(), level};
    }
  };

  void NextReachableFromCell(
      const LevelCoords& current,
      absl::flat_hash_set<LevelCoords>* reachable,
      std::vector<LevelCoords>* next) const {
    InsertIfReachableAndNew(current.Up(), reachable, next);
    InsertIfReachableAndNew(current.Down(), reachable, next);
    InsertIfReachableAndNew(current.Left(), reachable, next);
    InsertIfReachableAndNew(current.Right(), reachable, next);

    auto portal_it = portals_.find(current.coords);
    if (portal_it == portals_.end()) return;
    if (portal_it->second.outer && current.level == 0) return;

    const int level_delta = portal_it->second.outer ? -1 : 1;
    LevelCoords teleported{portal_it->second.coords,
                           current.level + level_delta};
    InsertIfReachableAndNew(teleported, reachable, next);
  }

  void InsertIfReachableAndNew(
      const LevelCoords candidate,
      absl::flat_hash_set<LevelCoords>* reachable,
      std::vector<LevelCoords>* next) const {
    if (!Inbounds(candidate.coords)) return;
    if (AtCoords(candidate.coords) == '.' &&
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

  // Returns label, coords of adjacent free space.
  std::optional<std::pair<std::array<char, 2>, CellCoords>> LabelAt(
      CellCoords coords) const {
    if (Inbounds(coords.Down()) && std::isupper(AtCoords(coords.Down()))) {
      if (Inbounds(coords.Up()) && AtCoords(coords.Up()) == '.') {
        return {{{AtCoords(coords), AtCoords(coords.Down())}, coords.Up()}};
      }
      if (Inbounds(coords.Down().Down()) &&
          AtCoords(coords.Down().Down()) == '.') {
        return {{{AtCoords(coords), AtCoords(coords.Down())},
                 coords.Down().Down()}};
      }
      std::cerr << "No open space adjacent to label\n";
      CHECK(false);
    }
    if (Inbounds(coords.Right()) && std::isupper(AtCoords(coords.Right()))) {
      if (Inbounds(coords.Left()) && AtCoords(coords.Left()) == '.') {
        return {{{AtCoords(coords), AtCoords(coords.Right())}, coords.Left()}};
      }
      if (Inbounds(coords.Right().Right()) &&
                   AtCoords(coords.Right().Right()) == '.') {
        return {{{AtCoords(coords), AtCoords(coords.Right())},
                 coords.Right().Right()}};
      }
      std::cerr << "No open space adjacent to label\n";
      CHECK(false);
    }
    return std::nullopt;
  }

  bool OuterEdge(CellCoords coords) const {
    if (coords.y < 3) return true;
    if (coords.y >= map_.size() - 3) return true;
    if (coords.x < 3) return true;
    if (coords.x >= map_[coords.y].size() - 3) return true;
    return false;
  }

  std::vector<std::string> map_;
  absl::flat_hash_map<CellCoords, TaggedPortal> portals_;
  CellCoords start_, finish_;
};

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  MazeMap maze_map(argv[1]);
  std::cout << maze_map.ShortestPath() << "\n";
  return 0;
}
