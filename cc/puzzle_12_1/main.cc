#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <numeric>
#include <string>
#include <vector>

#include "cc/util/check.h"

namespace {

class OneDimensionalSystem {
 public:
  OneDimensionalSystem() = default;

  bool operator==(const OneDimensionalSystem& other) const {
    return moons_.size() == other.moons_.size() &&
           std::equal(moons_.begin(), moons_.end(), other.moons_.begin());
  }

  bool operator!=(const OneDimensionalSystem& other) const {
    return !(*this == other);
  }

  void AddMoon(const char* desc, int dim) {
    moons_.emplace_back(desc, dim);
  }

  std::int64_t Period() const {
    std::int64_t steps = 0;
    OneDimensionalSystem future_sys = *this;
    do {
      ++steps;
      future_sys.Step();
    } while (*this != future_sys);
    return steps;
  }

 private:
  class Moon {
   public:
    explicit Moon(const char* desc, int dim) {
      int x, y, z;
      CHECK(3 == std::sscanf(desc, "<x=%d, y=%d, z=%d>", &x, &y, &z));
      switch (dim) {
        case 0:
          pos_ = x;
          break;
        case 1:
          pos_ = y;
          break;
        case 2:
          pos_ = z;
          break;
        default:
          std::cerr << "Invalid dimension\n";
          CHECK(false);
      }
    }

    bool operator==(const Moon& other) const {
      return pos_ == other.pos_ && velocity_ == other.velocity_;
    }

    void ApplyGravity(const Moon& other) {
      if (pos_ < other.pos_) {
        ++velocity_;
      } else if (pos_ > other.pos_) {
        --velocity_;
      }
    }

    void ApplyVelocity() {
      pos_ += velocity_;
    }

   private:
    int pos_ = 0;
    int velocity_ = 0;
  };

  void Step() {
    for (auto outer_moon_it = moons_.begin(); outer_moon_it != moons_.end();
         ++outer_moon_it) {
      for (auto inner_moon_it = moons_.begin(); inner_moon_it != moons_.end();
           ++inner_moon_it) {
        if (outer_moon_it == inner_moon_it) continue;
        outer_moon_it->ApplyGravity(*inner_moon_it);
      }
    }

    for (Moon& moon : moons_) {
      moon.ApplyVelocity();
    }
  }

  std::vector<Moon> moons_;
};

std::vector<OneDimensionalSystem> LoadSystems(const char* filename) {
  std::vector<OneDimensionalSystem> systems(3);

  std::ifstream stream(filename);
  CHECK(stream);
  std::string line;
  while (std::getline(stream, line)) {
    systems[0].AddMoon(line.c_str(), 0);
    systems[1].AddMoon(line.c_str(), 1);
    systems[2].AddMoon(line.c_str(), 2);
  }
  stream.close();
  return systems;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  std::vector<OneDimensionalSystem> systems = LoadSystems(argv[1]);
  CHECK(systems.size() == 3);

  std::int64_t period = systems.front().Period();
  for (auto system_it = systems.begin() + 1; system_it != systems.end();
       ++system_it) {
    period = std::lcm(period, system_it->Period());
  }

  std::cout << period<< "\n";
  return 0;
}
