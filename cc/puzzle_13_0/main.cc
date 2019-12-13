#include <iostream>

#include "cc/util/check.h"
#include "cc/util/intcode.h"

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  aoc2019::IntcodeMachine machine(aoc2019::ReadIntcodeProgram(argv[1]));
  aoc2019::IntcodeMachine::RunResult result = machine.Run();
  CHECK(result.state == aoc2019::IntcodeMachine::ExecState::kHalt);
  CHECK(result.outputs.size() % 3 == 0);
  int blocks = 0;
  while (!result.outputs.empty()) {
    result.outputs.pop_front();
    result.outputs.pop_front();
    if (result.outputs.front() == 2) {
      ++blocks;
    }
    result.outputs.pop_front();
  }
  std::cout << blocks << "\n";
  return 0;
}
