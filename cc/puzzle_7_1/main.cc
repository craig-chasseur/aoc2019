#include <algorithm>
#include <cstdint>
#include <deque>
#include <iostream>
#include <limits>
#include <vector>

#include "cc/util/check.h"
#include "cc/util/intcode.h"

namespace {

std::int64_t RunAmplifiers(const std::vector<std::int64_t>& program,
                           const std::vector<std::int64_t>& phase_settings) {
  std::vector<aoc2019::IntcodeMachine> amplifiers;
  for (const std::int64_t phase : phase_settings) {
    amplifiers.emplace_back(program);
    amplifiers.back().PushInputs({phase});
  }
  auto amp_it = amplifiers.begin();
  std::deque<std::int64_t> signals{0};
  for (;;) {
    amp_it->PushInputs(std::move(signals));
    aoc2019::IntcodeMachine::RunResult result = amp_it->Run();
    signals = std::move(result.outputs);
    if (result.state == aoc2019::IntcodeMachine::ExecState::kHalt &&
        amp_it + 1 == amplifiers.end()) {
      CHECK(!signals.empty());
      const int last_output = signals.front();
      signals.pop_front();
      CHECK(signals.empty());
      return last_output;
    }
    if (++amp_it == amplifiers.end()) {
      amp_it = amplifiers.begin();
    }
  }
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  std::vector<std::int64_t> program = aoc2019::ReadIntcodeProgram(argv[1]);
  std::vector<std::int64_t> phases{5, 6, 7, 8, 9};
  std::int64_t max_out = std::numeric_limits<std::int64_t>::min();
  do {
    max_out = std::max(max_out, RunAmplifiers(program, phases));
  } while (std::next_permutation(phases.begin(), phases.end()));
  std::cout << max_out << "\n";
  return 0;
}
