#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "absl/numeric/int128.h"
#include "cc/util/check.h"

namespace {

// Basic modular arithmetic routines.
// ----------------------------------

std::int64_t SafeTruncate(const absl::int128& bigval) {
  CHECK(absl::Int128High64(bigval) == 0);
  return absl::Int128Low64(bigval);
}

std::int64_t ModMul(std::int64_t a, std::int64_t b, std::int64_t mod) {
  absl::int128 res = (absl::int128(a) * absl::int128(b)) % absl::int128(mod);
  if (res < 0) res += mod;
  return SafeTruncate(res);
}

std::int64_t ModularInverse(std::int64_t a, std::int64_t b) {
  if (b == 1) return 1;
  std::int64_t b_init = b;
  std::int64_t x0 = 0;
  std::int64_t x1 = 1;
  while (a > 1) {
    std::int64_t q = a / b;
    std::int64_t tmp = b;
    b = a % b;
    a = tmp;
    tmp = x0;
    x0 = x1 - q * x0;
    x1 = tmp;
  }
  if (x1 < 0) x1 += b_init;
  return x1;
}

// Command descriptions.
// ---------------------

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

// Linear algebra.
// ---------------

// Coefficients of a linear polynomial: f(x) = a*x + b
struct Coefficients {
  std::int64_t a = 0;
  std::int64_t b = 0;

  std::int64_t EvalPolynomial(const std::int64_t x,
                              const std::int64_t mod) const {
    return (ModMul(a, x, mod) + b) % mod;
  }
};

Coefficients ReverseDealIntoNewStack(Coefficients coeff,
                                     std::int64_t num_cards) {
  coeff.a = -coeff.a;
  coeff.b = num_cards - coeff.b - 1;
  return coeff;
}

Coefficients ReverseCut(Coefficients coeff, int n, std::int64_t num_cards) {
  coeff.b = (coeff.b + n) % num_cards;
  return coeff;
}

Coefficients ReverseDealWithIncrement(Coefficients coeff, int n,
                                      std::int64_t num_cards) {
  const std::int64_t inv = ModularInverse(n, num_cards);
  coeff.a = ModMul(coeff.a, inv, num_cards);
  coeff.b = ModMul(coeff.b, inv, num_cards);
  return coeff;
}

Coefficients ReverseShuffleCommand(Coefficients coeff, std::int64_t num_cards,
                                   Command command) {
  switch (command.type) {
    case Command::Type::kDealIntoNewStack:
      return ReverseDealIntoNewStack(coeff, num_cards);
    case Command::Type::kCut:
      return ReverseCut(coeff, command.n, num_cards);
    case Command::Type::kDealWithIncrement:
      return ReverseDealWithIncrement(coeff, command.n, num_cards);
  }
}

Coefficients ReverseCommandSequence(Coefficients coeff, std::int64_t num_cards,
                                    const std::vector<Command>& commands) {
  for (auto cmd_it = commands.rbegin(); cmd_it != commands.rend(); ++cmd_it) {
    coeff = ReverseShuffleCommand(coeff, num_cards, *cmd_it);
  }
  return coeff;
}

// Raises the polynomial 'base' to the power 'exp' modulo 'mod'.
Coefficients ModExp(Coefficients base, std::int64_t exp, std::int64_t mod) {
  if (exp == 0) {
    return {1, 0};
  }
  if ((exp & 0x1) == 0) {
    return ModExp(Coefficients{ModMul(base.a, base.a, mod),
                               (ModMul(base.a, base.b, mod) + base.b) % mod},
                  exp >> 1, mod);
  }
  Coefficients tmp = ModExp(base, exp - 1, mod);
  return {ModMul(base.a, tmp.a, mod),
          (ModMul(base.a, tmp.b, mod) + base.b) % mod};
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

  // Compose all the commands (in reverse order) into a single polynomial.
  Coefficients coeff = ReverseCommandSequence({1, 0}, kNumCards, commands);

  // Raise the polynomial to 'kNumShuffles' power for repeated application.
  Coefficients exp_coeff = ModExp(coeff, kNumShuffles, kNumCards);

  // Evaluate polynomial to find the backtracked card at position 2020.
  std::cout << exp_coeff.EvalPolynomial(2020, kNumCards) << "\n";

  return 0;
}
