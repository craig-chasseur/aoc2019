#include <cctype>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
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

int GetMessageOffset(const std::vector<int>& signal) {
  return signal[0] * 1000000 +
         signal[1] * 100000 +
         signal[2] * 10000 +
         signal[3] * 1000 +
         signal[4] * 100 +
         signal[5] * 10 +
         signal[6];
}

std::vector<int> ApplyPartialFftPhase(
    const std::vector<int>& signal,
    const int message_offset) {
  std::vector<int> next(signal.size());
  int partial_sum = 0;
  for (int i = signal.size() - 1; i >= message_offset; --i) {
    next[i] = partial_sum = (partial_sum + signal[i]) % 10;
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
  const int message_offset = GetMessageOffset(signal);

  std::vector<int> bigsignal;
  bigsignal.reserve(signal.size() * 10000);
  for (int i = 0; i < 10000; ++i) {
    bigsignal.insert(bigsignal.end(), signal.begin(), signal.end());
  }

  for (int phase = 0; phase < 100; ++phase) {
    bigsignal = ApplyPartialFftPhase(bigsignal, message_offset);
  }

  for (int pos = message_offset; pos < message_offset + 8; ++pos) {
    std::cout << bigsignal[pos];
  }
  std::cout << "\n";

  return 0;
}
