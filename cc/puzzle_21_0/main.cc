#include <cstdint>
#include <iostream>
#include <vector>

#include "cc/util/intcode.h"

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  aoc2019::IntcodeMachine machine(aoc2019::ReadIntcodeProgram(argv[1]));

  // JumpScript solution reasoning:
  //   1. It's only possible to jump if D is ground.
  //   2. You *must* jump if A is a hole.
  //   3. If B is a hole, you must either jump now or in +1 turn.
  //   4. If C is a hole, you must either jump now, at +1 turn, or +2 turn.
  //   5. It is impossible to know if it will be safe to jump in +1 turn or +2
  //      turn. If you need to jump in the next couple of turns (i.e. A, B, or C
  //      is a hole) it is safe to jump now (i.e. D is ground), then you should;
  //      you will land on ground and you will not miss an opportune landing
  //      that you might have been able to make by waiting, since you would be
  //      able to reach the same spot by just walking forward.
  //   6. You should not jump if you are not compelled to by holes in the next
  //      3 spots. Doing so might land you in a position where both A and D are
  //      holes.
  //
  // This works out to J = (!A || !B || !C) && D
  // Applying De Morgan's laws: J = !(A && B && C) && D
  // Expressed in jumpscript this is:
  //
  // OR A J
  // AND B J
  // AND C J
  // NOT J J
  // AND D J
  // WALK

  machine.RunWithAsciiConsoleIO();
  return 0;
}
