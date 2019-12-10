#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
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
    int best = 0;
    for (auto aster_it = asteroids_.begin(); aster_it != asteroids_.end();
         ++aster_it) {
      best = std::max(best, CountVisibility(aster_it));
    }
    return best;
  }

 private:
  struct Asteroid {
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

    template <typename H>
    friend H AbslHashValue(H h, const Slope& slope) {
      return H::combine(std::move(h), slope.dx, slope.dy);
    }

    int dx = 0;
    int dy = 0;
  };

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
  std::cout << map.FindBestVisibility() << "\n";
  return 0;
}
