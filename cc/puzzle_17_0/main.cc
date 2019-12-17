#include <cstdint>
#include <deque>
#include <iostream>
#include <string>
#include <vector>

#include "cc/util/check.h"
#include "cc/util/intcode.h"

namespace {

std::vector<std::string> AsciiGrid(const std::deque<std::int64_t>& output) {
  std::vector<std::string> grid(1);
  for (const int64_t val : output) {
    if (val == '\n') {
      grid.emplace_back();
    } else {
      grid.back().push_back(static_cast<char>(val));
    }
  }
  if (grid.back().empty()) grid.pop_back();
  return grid;
}

int FindIntersections(const std::vector<std::string>& grid) {
  int sum = 0;
  for (int y = 1; y < grid.size() - 1; ++y) {
    for (int x = 1; x < grid[y].size() - 1; ++x) {
      if (grid[y][x] == '#' && grid[y - 1][x] == '#' && grid[y + 1][x] == '#' &&
          grid[y][x - 1] == '#' && grid[y][x + 1] == '#') {
        sum += x * y;
      }
    }
  }
  return sum;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  aoc2019::IntcodeMachine machine(aoc2019::ReadIntcodeProgram(argv[1]));
  aoc2019::IntcodeMachine::RunResult result = machine.Run();
  CHECK(result.state == aoc2019::IntcodeMachine::ExecState::kHalt);
  std::cout << FindIntersections(AsciiGrid(result.outputs)) << "\n";
  return 0;
}
