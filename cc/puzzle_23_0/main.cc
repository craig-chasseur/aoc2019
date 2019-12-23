#include <cstdint>
#include <iostream>
#include <vector>

#include "cc/util/check.h"
#include "cc/util/intcode.h"

namespace {

class Network {
 public:
  explicit Network(const std::vector<std::int64_t>& program,
                   const std::int64_t num_machines)
      : machines_(num_machines, aoc2019::IntcodeMachine(program)) {
    for (std::int64_t i = 0; i < num_machines; ++i) {
      machines_[i].PushInputs({i});
    }
  }

  std::int64_t Run() {
    for (;;) {
      for (aoc2019::IntcodeMachine& machine : machines_) {
        aoc2019::IntcodeMachine::RunResult result = machine.Run();
        CHECK(result.state ==
              aoc2019::IntcodeMachine::ExecState::kPendingInput);
        while (!result.outputs.empty()) {
          std::int64_t addr = result.outputs.front();
          result.outputs.pop_front();
          CHECK(!result.outputs.empty());
          std::int64_t x = result.outputs.front();
          result.outputs.pop_front();
          CHECK(!result.outputs.empty());
          std::int64_t y = result.outputs.front();
          result.outputs.pop_front();
          if (addr == 255) return y;
          if (addr >= 0 && addr < machines_.size()) {
            machines_[addr].PushInputs({x, y});
          }
        }
        machine.PushInputs({-1});
      }
    }
  }

 private:
  std::vector<aoc2019::IntcodeMachine> machines_;
};

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  Network network(aoc2019::ReadIntcodeProgram(argv[1]), 50);
  std::cout << network.Run() << "\n";
  return 0;
}
