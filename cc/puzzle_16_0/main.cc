#include <cctype>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "cc/util/check.h"

namespace {

std::vector<int> LoadSignal(const char* filename) {
  std::ifstream stream(filename);
  CHECK(stream);
  std::vector<int> pattern;
  char digit;
  while (stream >> digit && std::isdigit(digit)) {
    pattern.push_back(digit - '0');
  }
  stream.close();
  return pattern;
}

void PrintSignal(const std::vector<int>& signal) {
  for (int digit : signal) {
    std::cout << digit;
  }
  std::cout << "\n";
}

int PatternElement(int output_element, int input_element) {
  const int pattern_len = (output_element + 1) << 2;
  const int offset = (input_element + 1) % (pattern_len);
  switch (offset / (output_element + 1)) {
    case 0:
    case 2:
      return 0;
    case 1:
      return 1;
    case 3:
      return -1;
    default:
      std::cerr << "Invalid pattern offset";
      CHECK(false);
  }
}

std::vector<int> ApplyFftPhase(const std::vector<int>& signal) {
  std::vector<int> next(signal.size());
  for (int output_position = 0; output_position < next.size();
       ++output_position) {
    int sum = 0;
    for (int input_position = 0; input_position < signal.size();
         ++input_position) {
      sum += PatternElement(output_position, input_position) *
             signal[input_position];
    }
    next[output_position] = std::abs(sum) % 10;
  }
  return next;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  std::vector<int> signal = LoadSignal(argv[1]);
  for (int phase = 0; phase < 100; ++phase) {
    signal = ApplyFftPhase(signal);
  }
  PrintSignal(signal);
  return 0;
}
