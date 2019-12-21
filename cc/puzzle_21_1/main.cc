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
  //   5. The spot where you land after jumping must be viable. It's NOT viable
  //      if you must jump because the next spot (E) is a hole, but where you
  //      would land on the subsequent jump (H) is also a hole. You should only
  //      jump if E or H is ground.
  //   6. If B or C is a hole and it's not viable to jump right now, you must
  //      walk forward, which effectively shifts B -> A (then you must jump) or
  //      C -> B (you may jump if viable, otherwise walk forward again and
  //      there will be a hole at A and you must jump).
  //
  // This works out to J = !(A && B && C) && D && (E || H)
  // Expressed in jumpscript this is:
  //
  // OR A J
  // AND B J
  // AND C J
  // NOT J J
  // AND D J
  // OR E T
  // OR H T
  // AND T J
  // RUN

  machine.RunWithAsciiConsoleIO();
  return 0;
}
