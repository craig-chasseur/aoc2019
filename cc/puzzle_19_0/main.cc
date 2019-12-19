#include <cstdint>
#include <iostream>
#include <vector>

#include "cc/util/check.h"
#include "cc/util/intcode.h"

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  std::vector<std::int64_t> program = aoc2019::ReadIntcodeProgram(argv[1]);
  int count = 0;
  for (std::int64_t x = 0; x < 50; ++x) {
    for (std::int64_t y = 0; y < 50; ++y) {
      aoc2019::IntcodeMachine machine(program);
      machine.PushInputs({x, y});
      aoc2019::IntcodeMachine::RunResult result = machine.Run();
      CHECK(result.state == aoc2019::IntcodeMachine::ExecState::kHalt);
      CHECK(!result.outputs.empty());
      switch (result.outputs.front()) {
        case 0:
          break;
        case 1:
          ++count;
          break;
        default:
          std::cerr << "Invalid output code: " << result.outputs.front();
          CHECK(false);
      }
    }
  }
  std::cout << count << "\n";

  return 0;
}
