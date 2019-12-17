#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

#include "cc/util/intcode.h"

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  std::vector<std::int64_t> program = aoc2019::ReadIntcodeProgram(argv[1]);
  program[0] = 2;
  aoc2019::IntcodeMachine machine(std::move(program));
  machine.RunWithAsciiConsoleIO();
  return 0;
}
