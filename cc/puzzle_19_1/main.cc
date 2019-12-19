#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

#include "cc/util/check.h"
#include "cc/util/intcode.h"

namespace {

struct Coords {
  std::int64_t x = 0;
  std::int64_t y = 0;
};

bool InBeam(std::vector<std::int64_t> program, const Coords& coords) {
  aoc2019::IntcodeMachine machine(std::move(program));
  machine.PushInputs({coords.x, coords.y});
  aoc2019::IntcodeMachine::RunResult result = machine.Run();
  CHECK(result.state == aoc2019::IntcodeMachine::ExecState::kHalt);
  CHECK(!result.outputs.empty());
  switch (result.outputs.front()) {
    case 0:
      return false;
    case 1:
      return true;
    default:
      std::cerr << "Invalid output code: " << result.outputs.front();
      CHECK(false);
  }
}

Coords NextBottomEdge(const std::vector<std::int64_t>& program,
                      const Coords& coords) {
  Coords next{coords.x, coords.y + 1};
  while (!InBeam(program, next)) {
    ++next.x;
  }
  return next;
}

Coords StartPos(const std::vector<std::int64_t>& program) {
  // Find the left edge of the beam at y = 99.
  Coords start{0, 99};
  while (!InBeam(program, start)) {
    ++start.x;
  }
  return start;
}

Coords FindClosest(const std::vector<std::int64_t>& program) {
  Coords bottom_left;
  for (bottom_left = StartPos(program);
       !InBeam(program, Coords{bottom_left.x + 99, bottom_left.y - 99});
       bottom_left = NextBottomEdge(program, bottom_left)) {
  }
  return Coords{bottom_left.x, bottom_left.y - 99};
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  std::vector<std::int64_t> program = aoc2019::ReadIntcodeProgram(argv[1]);
  Coords closest = FindClosest(program);
  std::cout << (closest.x * 10000 + closest.y) << "\n";
  return 0;
}
