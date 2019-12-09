#include <iostream>

#include "cc/util/check.h"
#include "cc/util/intcode.h"

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  aoc2019::IntcodeMachine machine(aoc2019::ReadIntcodeProgram(argv[1]));
  machine.RunWithConsoleIO();
  return 0;
}
