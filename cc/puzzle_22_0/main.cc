#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "cc/util/check.h"

namespace {

std::vector<std::string> LoadShuffleCommands(const char* filename) {
  std::vector<std::string> commands;
  std::ifstream stream(filename);
  CHECK(stream);
  std::string cmd;
  while (std::getline(stream, cmd)) {
    if (cmd.back() == '\n') cmd.pop_back();
    commands.emplace_back(std::move(cmd));
  }
  stream.close();
  return commands;
}

std::vector<int> DealIntoNewStack(std::vector<int> deck) {
  std::reverse(deck.begin(), deck.end());
  return deck;
}

std::vector<int> Cut(std::vector<int> deck, int n) {
  if (n < 0) n = deck.size() + n;
  std::rotate(deck.begin(), deck.begin() + n, deck.end());
  return deck;
}

std::vector<int> DealWithIncrement(std::vector<int> deck, const int n) {
  std::vector<int> dealt(deck.size(), 0);
  for (int deck_idx = 0; deck_idx < deck.size(); ++deck_idx) {
    dealt[(deck_idx * n) % deck.size()] = deck[deck_idx];
  }
  return dealt;
}

std::vector<int> RunShuffleCommand(std::vector<int> deck,
                                   const std::string& command) {
  if (command == "deal into new stack") {
    return DealIntoNewStack(deck);
  }
  int n;
  if (std::sscanf(command.c_str(), "cut %d", &n) == 1) {
    return Cut(deck, n);
  }
  CHECK(std::sscanf(command.c_str(), "deal with increment %d", &n) == 1);
  return DealWithIncrement(deck, n);
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }

  std::vector<int> deck(10007, 0);
  std::iota(deck.begin(), deck.end(), 0);

  std::vector<std::string> commands = LoadShuffleCommands(argv[1]);
  for (const std::string& cmd : commands) {
    deck = RunShuffleCommand(deck, cmd);
  }

  auto loc = std::find(deck.begin(), deck.end(), 2019);
  CHECK(loc != deck.end());
  std::cout << (loc - deck.begin()) << "\n";
  return 0;
}
