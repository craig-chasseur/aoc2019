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
  explicit Horizontal(int start_path, int x_start, int x_end, int y)
      : start_path_(start_path), x_start_(x_start), x_end_(x_end), y_(y) {}

 private:
  friend class Vertical;

  int start_path_, x_start_, x_end_, y_;
};

class Vertical {
 public:
  explicit Vertical(int start_path, int x, int y_start, int y_end)
      : start_path_(start_path), x_(x), y_start_(y_start), y_end_(y_end) {
  }

  std::optional<int> Intersect(const Horizontal& hor) const {
    if (x_ < std::min(hor.x_start_, hor.x_end_)) return std::nullopt;
    if (x_ > std::max(hor.x_start_, hor.x_end_)) return std::nullopt;
    if (hor.y_ < std::min(y_start_, y_end_)) return std::nullopt;
    if (hor.y_ > std::max(y_start_, y_end_)) return std::nullopt;
    if (x_ == 0 && hor.y_ == 0) return std::nullopt;
    return start_path_ + std::abs(hor.y_ - y_start_) +
           hor.start_path_ + std::abs(x_ - hor.x_start_);
  }

 private:
  int start_path_, x_, y_start_, y_end_;
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
  int sum_distance = 0;
  for (absl::string_view move : absl::StrSplit(path, ",")) {
    CHECK(!move.empty());
    const char direction = move[0];
    move.remove_prefix(1);
    int distance;
    CHECK(absl::SimpleAtoi(move, &distance));
    switch (direction) {
      case 'R':
        segments.AddHorizontal(Horizontal(sum_distance, x, x + distance, y));
        x += distance;
        sum_distance += distance;
        break;
      case 'L':
        segments.AddHorizontal(Horizontal(sum_distance, x, x - distance, y));
        x -= distance;
        sum_distance += distance;
        break;
      case 'U':
        segments.AddVertical(Vertical(sum_distance, x, y, y + distance));
        y += distance;
        sum_distance += distance;
        break;
      case 'D':
        segments.AddVertical(Vertical(sum_distance, x, y, y - distance));
        y -= distance;
        sum_distance += distance;
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
