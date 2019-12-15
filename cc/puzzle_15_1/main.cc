#include <cstdint>
#include <deque>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "absl/hash/hash.h"
#include "cc/util/check.h"
#include "cc/util/intcode.h"

namespace {

struct Position {
  int x = 0;
  int y = 0;

  bool operator==(const Position& other) const {
    return x == other.x && y == other.y;
  }

  template <typename H>
  friend H AbslHashValue(H h, const Position& pos) {
    return H::combine(std::move(h), pos.x, pos.y);
  }

  Position Move(std::int64_t direction) const {
    switch (direction) {
      case 1:
        return Position{x, y - 1};
      case 2:
        return Position{x, y + 1};
      case 3:
        return Position{x - 1, y};
      case 4:
        return Position{x + 1, y};
      default:
        std::cerr << "Invalid direction: " << direction;
        CHECK(false);
    }
  }

  static Position FromMoveSequence(const std::deque<std::int64_t> seq) {
    Position pos;
    for (const std::int64_t move : seq) {
      pos = pos.Move(move);
    }
    return pos;
  }
};

// Returns the shortest path to the oxygen system.
std::deque<std::int64_t>
    BfsOxygenSearch(const std::vector<std::int64_t>& program) {
  absl::flat_hash_set<Position> horizon{{0, 0}};
  std::vector<std::deque<std::int64_t>> paths{{1}, {2}, {3}, {4}};
  for (;;) {
    std::vector<std::deque<std::int64_t>> next_paths;
    for (std::deque<std::int64_t>& candidate_path : paths) {
      aoc2019::IntcodeMachine droid(program);
      droid.PushInputs(candidate_path);
      aoc2019::IntcodeMachine::RunResult result = droid.Run();
      CHECK(result.state == aoc2019::IntcodeMachine::ExecState::kPendingInput);
      CHECK(!result.outputs.empty());
      if (result.outputs.back() == 2) {
        return std::move(candidate_path);
      }
      if (result.outputs.back() == 1) {
        Position current_pos = Position::FromMoveSequence(candidate_path);
        for (std::int64_t next_move : {1, 2, 3, 4}) {
          if (horizon.insert(current_pos.Move(next_move)).second) {
            next_paths.emplace_back(candidate_path);
            next_paths.back().push_back(next_move);
          }
        }
      }
    }
    paths = std::move(next_paths);
  }
}

int TimeToOxygenate(const std::vector<std::int64_t>& program,
                    std::deque<int64_t> path_to_o2) {
  absl::flat_hash_set<Position> oxygenated{
      Position::FromMoveSequence(path_to_o2)};
  std::vector<std::deque<std::int64_t>> paths{std::move(path_to_o2)};
  int cycles = 0;
  while (!paths.empty()) {
    ++cycles;
    std::vector<std::deque<std::int64_t>> next_paths;
    for (std::deque<std::int64_t>& candidate_path : paths) {
      aoc2019::IntcodeMachine droid(program);
      droid.PushInputs(candidate_path);
      aoc2019::IntcodeMachine::RunResult result = droid.Run();
      CHECK(result.state == aoc2019::IntcodeMachine::ExecState::kPendingInput);
      CHECK(!result.outputs.empty());
      if (result.outputs.back() != 0) {
        Position current_pos = Position::FromMoveSequence(candidate_path);
        for (std::int64_t next_move : {1, 2, 3, 4}) {
          if (oxygenated.insert(current_pos.Move(next_move)).second) {
            next_paths.emplace_back(candidate_path);
            next_paths.back().push_back(next_move);
          }
        }
      }
    }
    paths = std::move(next_paths);
  }
  return cycles - 1;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  std::vector<std::int64_t> program = aoc2019::ReadIntcodeProgram(argv[1]);
  std::cout << TimeToOxygenate(program, BfsOxygenSearch(program)) << "\n";
  return 0;
}
