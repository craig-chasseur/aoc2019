#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "cc/util/check.h"

namespace {

struct Command {
  enum class Type {
    kDealIntoNewStack,
    kCut,
    kDealWithIncrement,
  };

  Type type;
  int n = 0;
};

std::vector<Command> LoadShuffleCommands(const char* filename) {
  std::vector<Command> commands;
  std::ifstream stream(filename);
  CHECK(stream);
  std::string cmd;
  while (std::getline(stream, cmd)) {
    if (cmd.back() == '\n') cmd.pop_back();
    if (cmd == "deal into new stack") {
      commands.emplace_back(Command{Command::Type::kDealIntoNewStack, 0});
      continue;
    }
    int n;
    if (std::sscanf(cmd.c_str(), "cut %d", &n) == 1) {
      commands.emplace_back(Command{Command::Type::kCut, n});
      continue;
    }
    CHECK(std::sscanf(cmd.c_str(), "deal with increment %d", &n) == 1);
    commands.emplace_back(Command{Command::Type::kDealWithIncrement, n});
  }
  stream.close();
  return commands;
}

std::int64_t ReverseDealIntoNewStack(std::int64_t num_cards, std::int64_t pos) {
  return num_cards - pos - 1;
}

std::int64_t ReverseCut(std::int64_t num_cards, int n, std::int64_t pos) {
  pos += n;
  if (pos < 0) return num_cards + pos;
  return pos % num_cards;
}

std::int64_t ReverseDealWithIncrement(
    std::int64_t num_cards, const int n, std::int64_t pos) {
  while (pos % n != 0) {
    pos += num_cards;
  }
  return pos / n;
}

std::int64_t ReverseShuffleCommand(std::int64_t num_cards, std::int64_t pos,
                                   Command command) {
  switch (command.type) {
    case Command::Type::kDealIntoNewStack:
      return ReverseDealIntoNewStack(num_cards, pos);
    case Command::Type::kCut:
      return ReverseCut(num_cards, command.n, pos);
    case Command::Type::kDealWithIncrement:
      return ReverseDealWithIncrement(num_cards, command.n, pos);
  }
}

std::int64_t ReverseCommandSequence(std::int64_t num_cards, std::int64_t pos,
                                    const std::vector<Command>& commands) {
  for (auto cmd_it = commands.rbegin(); cmd_it != commands.rend(); ++cmd_it) {
    pos = ReverseShuffleCommand(num_cards, pos, *cmd_it);
  }
  return pos;
}

}  // namespace

int main(int argc, char** argv) {
  static constexpr std::int64_t kNumCards = INT64_C(119315717514047);
  static constexpr std::int64_t kNumShuffles = INT64_C(101741582076661);

  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }

  std::vector<Command> commands = LoadShuffleCommands(argv[1]);

  std::int64_t pos = 2020;
  for (std::int64_t i = 0; i < kNumShuffles; ++i) {
    pos = ReverseCommandSequence(kNumCards, pos, commands);
  }

  std::cout << pos << "\n";
  return 0;
}
