#include <cstdint>
#include <deque>
#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "cc/util/check.h"
#include "cc/util/intcode.h"

namespace {

class Grid {
 public:
  explicit Grid(const std::deque<std::int64_t>& output) {
    grid_.emplace_back();
    for (const int64_t val : output) {
      if (val == '\n') {
        grid_.emplace_back();
      } else {
        grid_.back().push_back(static_cast<char>(val));
      }
    }
    if (grid_.back().empty()) grid_.pop_back();
    CHECK(!grid_.empty());
  }

  // This uses a few heuristics to choose one "canonical" path:
  //     1. We always go straight through intersections rather than turning at
  //        them. Put another way, we go as far as possible in a line before
  //        turning. It appears that the scaffolding has two "ends" and a series
  //        of intersections, so this will allow visiting the entire scaffolding
  //        without backtracking.
  //     2. We always alternate between turning and moving forward. We do not
  //        consider paths where a longer line segment is composed of multiple
  //        smaller moves, nor where multiple turns are executed without moving
  //        (e.g. 3 lefts to make a right).
  std::vector<std::string> CalculatePath() const {
    VacuumRobot bot = LocateVacuumRobot();
    std::vector<std::string> path;
    for (;;) {
      std::optional<std::string> turn = NextTurn(&bot);
      if (!turn.has_value()) break;
      path.emplace_back(std::move(turn).value());
      path.emplace_back(NextMove(&bot));
    }
    return path;
  }

 private:
  enum class Facing {
    kUp,
    kRight,
    kDown,
    kLeft,
  };

  struct VacuumRobot {
    Facing facing;
    int x;
    int y;
  };

  VacuumRobot LocateVacuumRobot() const {
    for (int y = 0; y < grid_.size(); ++y) {
      for (int x = y; x < grid_[y].size(); ++x) {
        switch (grid_[y][x]) {
          case '^':
            return VacuumRobot{Facing::kUp, x, y};
          case '>':
            return VacuumRobot{Facing::kRight, x, y};
          case 'v':
            return VacuumRobot{Facing::kDown, x, y};
          case '<':
            return VacuumRobot{Facing::kLeft, x, y};
          default:
            break;
        }
      }
    }
    std::cerr << "Couldn't find vacuum robot\n";
    CHECK(false);
  }

  std::optional<std::string> NextTurn(VacuumRobot* bot) const {
    switch (bot->facing) {
      case Facing::kUp:
        if (bot->x > 0 && grid_[bot->y][bot->x - 1] == '#') {
          bot->facing = Facing::kLeft;
          return "L";
        }
        if (bot->x < grid_[bot->y].size() - 1 &&
            grid_[bot->y][bot->x + 1] == '#') {
          bot->facing = Facing::kRight;
          return "R";
        }
        return std::nullopt;
      case Facing::kRight:
        if (bot->y > 0 && grid_[bot->y - 1][bot->x] == '#') {
          bot->facing = Facing::kUp;
          return "L";
        }
        if (bot->y < grid_.size() - 1 && grid_[bot->y + 1][bot->x] == '#') {
          bot->facing = Facing::kDown;
          return "R";
        }
        return std::nullopt;
      case Facing::kDown:
        if (bot->x > 0 && grid_[bot->y][bot->x - 1] == '#') {
          bot->facing = Facing::kLeft;
          return "R";
        }
        if (bot->x < grid_[bot->y].size() - 1 &&
            grid_[bot->y][bot->x + 1] == '#') {
          bot->facing = Facing::kRight;
          return "L";
        }
        return std::nullopt;
      case Facing::kLeft:
        if (bot->y > 0 && grid_[bot->y - 1][bot->x] == '#') {
          bot->facing = Facing::kUp;
          return "R";
        }
        if (bot->y < grid_.size() - 1 && grid_[bot->y + 1][bot->x] == '#') {
          bot->facing = Facing::kDown;
          return "L";
        }
        return std::nullopt;
    }
  }

  std::string NextMove(VacuumRobot* bot) const {
    switch (bot->facing) {
      case Facing::kUp: {
        const int init_y = bot->y;
        while (bot->y > 0 && grid_[bot->y - 1][bot->x] == '#') {
          --bot->y;
        }
        return absl::StrCat(init_y - bot->y);
      }
      case Facing::kRight: {
        const int init_x = bot->x;
        while (bot->x < grid_[bot->y].size() - 1 &&
               grid_[bot->y][bot->x + 1] == '#') {
          ++bot->x;
        }
        return absl::StrCat(bot->x - init_x);
      }
      case Facing::kDown: {
        const int init_y = bot->y;
        while (bot->y < grid_.size() - 1 &&
               grid_[bot->y + 1][bot->x] == '#') {
          ++bot->y;
        }
        return absl::StrCat(bot->y - init_y);
      }
      case Facing::kLeft: {
        const int init_x = bot->x;
        while (bot->x > 0 && grid_[bot->y][bot->x - 1] == '#') {
          --bot->x;
        }
        return absl::StrCat(init_x - bot->x);
      }
    }
  }

  std::vector<std::string> grid_;
};

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  std::vector<std::int64_t> program = aoc2019::ReadIntcodeProgram(argv[1]);
  aoc2019::IntcodeMachine map_machine(program);
  aoc2019::IntcodeMachine::RunResult map_result = map_machine.Run();
  CHECK(map_result.state == aoc2019::IntcodeMachine::ExecState::kHalt);

  Grid grid(map_result.outputs);
  std::cout << absl::StrJoin(grid.CalculatePath(), ",") << "\n";

  return 0;
}
