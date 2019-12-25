#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "absl/strings/str_cat.h"
#include "cc/util/check.h"
#include "cc/util/intcode.h"

namespace {

// I came up with this sequence by manually exploring the ship. It collects all
// the items that are safe to pick up and winds up at the security checkpoint.
constexpr char kCollectSequence[] =
    "north\n"
    "take candy cane\n"
    "south\n"
    "south\n"
    "take fuel cell\n"
    "south\n"
    "take manifold\n"
    "north\n"
    "north\n"
    "west\n"
    "take mutex\n"
    "south\n"
    "south\n"
    "take coin\n"
    "west\n"
    "take dehydrated water\n"
    "south\n"
    "take prime number\n"
    "north\n"
    "east\n"
    "north\n"
    "east\n"
    "take cake\n"
    "north\n"
    "west\n"
    "south\n";

// All the items in the ship that are safe to pick up.
constexpr const char* kItems[] = {
    "cake",
    "candy cane",
    "coin",
    "dehydrated water",
    "fuel cell",
    "manifold",
    "mutex",
    "prime number"};

// Generates a command sequence that:
//     1. Drops any held items.
//     2. Picks up items indicated by the lower-order 8 bits of 'code'.
//     3. Tries to exit the security checkpoint to the west.
std::deque<int64_t> TryItems(std::uint32_t code) {
  std::string commands;
  for (const char* item : kItems) {
    absl::StrAppend(&commands, "drop ", item, "\n");
  }
  for (int i = 0; i < (sizeof(kItems) / sizeof(kItems[0])); ++i) {
    if (code & (1u << i)) {
      absl::StrAppend(&commands, "take ", kItems[i], "\n");
    }
  }
  absl::StrAppend(&commands, "west\n");
  return std::deque<int64_t>(commands.begin(), commands.end());
};

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  aoc2019::IntcodeMachine machine(aoc2019::ReadIntcodeProgram(argv[1]));

  machine.PushInputs(std::deque<std::int64_t>(
      kCollectSequence, kCollectSequence + sizeof(kCollectSequence) - 1));
  aoc2019::IntcodeMachine::RunResult result = machine.Run();
  CHECK(result.state == aoc2019::IntcodeMachine::ExecState::kPendingInput);

  for (std::uint32_t itemcode = 0; itemcode < 256; ++itemcode) {
    machine.PushInputs(TryItems(itemcode));
    result = machine.Run();
    if (result.state == aoc2019::IntcodeMachine::ExecState::kHalt) {
      std::cout << std::string(result.outputs.begin(), result.outputs.end());
      return 0;
    }
  }

  std::cerr << "Unable to clear security checkpoint\n";
  return 1;
}
