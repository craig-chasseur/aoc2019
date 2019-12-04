#include <cstdint>
#include <fstream>
#include <iostream>
#include <numeric>
#include <vector>

namespace {

std::vector<std::int64_t> ReadInputs(const char* filename) {
  std::vector<std::int64_t> values;
  std::int64_t buffer;
  std::ifstream stream(filename);
  while (stream >> buffer) {
    values.push_back(buffer);
  }
  stream.close();
  return values;
}

std::int64_t CalculateFuel(std::int64_t mass) {
  return (mass / 3) - 2;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  const std::vector<std::int64_t> values = ReadInputs(argv[1]);
  const std::int64_t total_fuel = std::accumulate(
      values.begin(), values.end(), std::int64_t{0},
      [](int64_t fuel_sum, int64_t mass) {
        return fuel_sum + CalculateFuel(mass);
      });
  std::cout << total_fuel << "\n";
  return 0;
}
