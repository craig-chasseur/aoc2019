#include <algorithm>
#include <cstdint>
#include <deque>
#include <iostream>
#include <limits>
#include <vector>

#include "cc/util/check.h"
#include "cc/util/intcode.h"

namespace {

std::int64_t RunAmplifiers(std::vector<std::int64_t> program,
                           const std::vector<std::int64_t>& phase_settings) {
  std::deque<std::int64_t> output{0};
  for (const std::int64_t phase : phase_settings) {
    output.push_front(phase);
    aoc2019::IntcodeMachine machine(program);
    machine.PushInputs(std::move(output));
    aoc2019::IntcodeMachine::RunResult result = machine.Run();
    CHECK(result.state == aoc2019::IntcodeMachine::ExecState::kHalt);
    output = std::move(result.outputs);
  }
  CHECK(output.size() == 1);
  return output.front();
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  std::vector<std::int64_t> program = aoc2019::ReadIntcodeProgram(argv[1]);
  std::vector<std::int64_t> phases{0, 1, 2, 3, 4};
  std::int64_t max_out = std::numeric_limits<std::int64_t>::min();
  do {
    max_out = std::max(max_out, RunAmplifiers(program, phases));
  } while (std::next_permutation(phases.begin(), phases.end()));
  std::cout << max_out << "\n";
  return 0;
}
