#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "cc/util/check.h"

namespace {

class Horizontal {
 public:
  explicit Horizontal(int xmin, int xmax, int y)
      : xmin_(xmin), xmax_(xmax), y_(y) {
    CHECK(xmin_ < xmax_);
  }

 private:
  friend class Vertical;

  int xmin_, xmax_, y_;
};

class Vertical {
 public:
  explicit Vertical(int x, int ymin, int ymax)
      : x_(x), ymin_(ymin), ymax_(ymax) {
    CHECK(ymin_ < ymax_);
  }

  std::optional<int> Intersect(const Horizontal& hor) const {
    if (x_ < hor.xmin_) return std::nullopt;
    if (x_ > hor.xmax_) return std::nullopt;
    if (hor.y_ < ymin_) return std::nullopt;
    if (hor.y_ > ymax_) return std::nullopt;
    if (x_ == 0 && hor.y_ == 0) return std::nullopt;
    return std::abs(x_) + std::abs(hor.y_);
  }

 private:
  int x_, ymin_, ymax_;
};

class WireSegments {
 public:
  WireSegments() = default;

  void AddHorizontal(Horizontal hor) {
    horizontals_.emplace_back(std::move(hor));
  }

  void AddVertical(Vertical ver) {
    verticals_.emplace_back(std::move(ver));
  }

  std::optional<int> FindClosestIntersection(const WireSegments& other) const {
    std::optional<int> closest = std::nullopt;
    auto update_closest = [&closest](std::optional<int> intersection) {
      if (!intersection.has_value()) return;
      if (!closest.has_value()) {
        closest = intersection;
        return;
      }
      closest = std::min(closest.value(), intersection.value());
    };

    for (const Horizontal& hor : horizontals_) {
      for (const Vertical& ver : other.verticals_) {
        update_closest(ver.Intersect(hor));
      }
    }
    for (const Horizontal& hor : other.horizontals_) {
      for (const Vertical& ver : verticals_) {
        update_closest(ver.Intersect(hor));
      }
    }

    return closest;
  }

 private:
  std::vector<Horizontal> horizontals_;
  std::vector<Vertical> verticals_;
};

WireSegments ParsePath(absl::string_view path) {
  WireSegments segments;
  int x = 0;
  int y = 0;
  for (absl::string_view move : absl::StrSplit(path, ",")) {
    CHECK(!move.empty());
    const char direction = move[0];
    move.remove_prefix(1);
    int distance;
    CHECK(absl::SimpleAtoi(move, &distance));
    switch (direction) {
      case 'R':
        segments.AddHorizontal(Horizontal(x, x + distance, y));
        x += distance;
        break;
      case 'L':
        segments.AddHorizontal(Horizontal(x - distance, x, y));
        x -= distance;
        break;
      case 'U':
        segments.AddVertical(Vertical(x, y, y + distance));
        y += distance;
        break;
      case 'D':
        segments.AddVertical(Vertical(x, y - distance, y));
        y -= distance;
        break;
      default:
        CHECK(false);
    }
  }
  return segments;
}

std::vector<std::string> ReadPaths(const char* filename) {
  std::vector<std::string> paths;
  std::ifstream stream(filename);
  std::string line;
  while (std::getline(stream, line)) {
    paths.emplace_back(std::move(line));
  }
  stream.close();
  return paths;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  std::vector<std::string> paths = ReadPaths(argv[1]);
  CHECK(paths.size() == 2);
  WireSegments wire_a = ParsePath(paths[0]);
  WireSegments wire_b = ParsePath(paths[1]);
  std::optional<int> closest_intersection =
      wire_a.FindClosestIntersection(wire_b);
  if (closest_intersection.has_value()) {
    std::cout << closest_intersection.value() << "\n";
  } else {
    std::cout << "NO INTERSECTION FOUND\n";
  }
  return 0;
}
