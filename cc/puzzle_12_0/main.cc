#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <numeric>
#include <string>
#include <vector>

#include "cc/util/check.h"

namespace {

class System {
 public:
  System() = default;

  void AddMoon(const char* desc) {
    moons_.emplace_back(desc);
  }

  void RunSteps(int n) {
    for (int i = 0; i < n; ++i) {
      Step();
    }
  }

  int TotalEnergy() const {
    return std::accumulate(
        moons_.begin(), moons_.end(), 0,
        [](int sum, const Moon& moon) { return sum + moon.Energy(); });
  }

 private:
  class Moon {
   public:
    explicit Moon(const char* desc) {
      CHECK(3 == std::sscanf(desc, "<x=%d, y=%d, z=%d>", &x_, &y_, &z_));
    }

    int Energy() const {
      return (std::abs(x_) + std::abs(y_) + std::abs(z_)) *
             (std::abs(vx_) + std::abs(vy_) + std::abs(vz_));
    }

    void ApplyGravity(const Moon& other) {
      if (x_ < other.x_) {
        ++vx_;
      } else if (x_ > other.x_) {
        --vx_;
      }

      if (y_ < other.y_) {
        ++vy_;
      } else if (y_ > other.y_) {
        --vy_;
      }

      if (z_ < other.z_) {
        ++vz_;
      } else if (z_ > other.z_) {
        --vz_;
      }
    }

    void ApplyVelocity() {
      x_ += vx_;
      y_ += vy_;
      z_ += vz_;
    }

   private:
    int x_ = 0;
    int y_ = 0;
    int z_ = 0;

    int vx_ = 0;
    int vy_ = 0;
    int vz_ = 0;
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

System LoadSystem(const char* filename) {
  System system;

  std::ifstream stream(filename);
  CHECK(stream);
  std::string line;
  while (std::getline(stream, line)) {
    system.AddMoon(line.c_str());
  }
  stream.close();
  return system;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  System system = LoadSystem(argv[1]);
  system.RunSteps(1000);
  std::cout << system.TotalEnergy() << "\n";
  return 0;
}
