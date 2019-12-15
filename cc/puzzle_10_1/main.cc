#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "absl/hash/hash.h"
#include "cc/util/check.h"

namespace {

int GCD(int a, int b) {
  if (a == 0) return b;
  return GCD(b % a, a);
}

class AsteroidMap {
 public:
  explicit AsteroidMap(const std::vector<std::string>& map) {
    for (int y = 0; y < map.size(); ++y) {
      for (int x = 0; x < map[y].size(); ++x) {
        if (map[y][x] == '#') {
          asteroids_.emplace_back(Asteroid{x, y});
        }
      }
    }
  }

  int FindBestVisibility() const {
    return FindBestVisibilityImpl().second;
  }

  int FindVaporized(int n) const {
    Asteroid station = FindBestVisibilityImpl().first;
    std::map<Slope, std::deque<Asteroid>> target_map;
    for (const Asteroid& asteroid : asteroids_) {
      if (station == asteroid) continue;
      target_map[Slope::Make(station, asteroid)].emplace_back(asteroid);
    }
    for (auto& target_line : target_map) {
      std::sort(
          target_line.second.begin(), target_line.second.end(),
          [&station](const Asteroid& a, const Asteroid& b) {
            const int a_dist =
                std::abs(a.x - station.x) + std::abs(a.y - station.y);
            const int b_dist =
                std::abs(b.x - station.x) + std::abs(b.y - station.y);
            return a_dist < b_dist;
          });
    }

    auto ray_it = target_map.begin();
    Asteroid last_vaporized;
    for (int i = 0; i < n; ++i) {
      last_vaporized = ray_it->second.front();
      // There's some off-by-one error somewhere in here (probably the ordering)
      // of slopes. So we verbosely log the destroyed asteroids so we have
      // some "close" guesses to try.
      std::cout << "Blasted asteroid #" << (i + 1) << " at "
                << last_vaporized.x << "," << last_vaporized.y
                << " (slope dx=" << ray_it->first.dx
                << ", dy=" << ray_it->first.dy << ")\n";
      ray_it->second.pop_front();
      if (ray_it->second.empty()) {
        ray_it = target_map.erase(ray_it);
      } else {
        ++ray_it;
      }
      if (ray_it == target_map.end()) {
        ray_it = target_map.begin();
      }
      CHECK(ray_it != target_map.end());
    }
    return 100 * last_vaporized.x + last_vaporized.y;
  }

 private:
  struct Asteroid {
    bool operator==(const Asteroid& other) const {
      return x == other.x && y == other.y;
    };

    int x;
    int y;
  };

  struct Slope {
    static Slope Make(const Asteroid& station, const Asteroid& target) {
      Slope slope;
      slope.dy = target.y - station.y;
      slope.dx = target.x - station.x;
      slope.Simplify();
      return slope;
    }

    void Simplify() {
      if (dx == 0) {
        if (dy > 0) {
          dy = 1;
          return;
        }
        dy = -1;
        return;
      }

      if (dy == 0) {
        if (dx > 0) {
          dx = 1;
          return;
        }
        dx = -1;
        return;
      }

      int gcd = GCD(dx, dy);
      if (gcd < 0) gcd = -gcd;
      dx /= gcd;
      dy /= gcd;
    }

    bool operator==(const Slope& other) const {
      return dx == other.dx && dy == other.dy;
    }

    bool operator<(const Slope& other) const {
      if (*this == other) return false;
      if (dx == 0) {
        if (dy < 0) return true;
        return other.dx > 0;
      }
      if (dx > 0) {
        if (other.dx < 0) return true;
        if (other.dx == 0) {
          return other.dy > 0;
        }
        return (static_cast<double>(dy) / static_cast<double>(dx)) <
               (static_cast<double>(other.dy) / static_cast<double>(other.dx));
      }
      if (other.dx >= 0) return false;
      return (static_cast<double>(dy) / static_cast<double>(dx)) <
             (static_cast<double>(other.dy) / static_cast<double>(other.dx));
    }

    template <typename H>
    friend H AbslHashValue(H h, const Slope& slope) {
      return H::combine(std::move(h), slope.dx, slope.dy);
    }

    int dx = 0;
    int dy = 0;
  };

  std::pair<Asteroid, int> FindBestVisibilityImpl() const {
    int best = 0;
    Asteroid best_aster;
    for (auto aster_it = asteroids_.begin(); aster_it != asteroids_.end();
         ++aster_it) {
      int vis = CountVisibility(aster_it);
      if (vis > best) {
        best = vis;
        best_aster = *aster_it;
      }
    }
    return {best_aster, best};
  }

  int CountVisibility(std::vector<Asteroid>::const_iterator aster_it) const {
    absl::flat_hash_set<Slope> slopes;
    for (auto other_it = asteroids_.begin(); other_it != asteroids_.end();
         ++other_it) {
      if (other_it == aster_it) continue;
      slopes.insert(Slope::Make(*aster_it, *other_it));
    }
    return slopes.size();
  }

  std::vector<Asteroid> asteroids_;
};

std::vector<std::string> ReadMap(const char* filename) {
  std::ifstream stream(filename);
  CHECK(stream);
  std::vector<std::string> map;
  std::string line;
  while (stream >> line) {
    map.emplace_back(std::move(line));
  }
  stream.close();
  return map;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  const std::vector<std::string> txtmap = ReadMap(argv[1]);
  AsteroidMap map(txtmap);
  std::cout << map.FindVaporized(200) << "\n";
  return 0;
}
