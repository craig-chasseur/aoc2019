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
  std::int64_t fuel_mass = 0;
  std::int64_t incremental_mass = mass;
  while (incremental_mass > 0) {
    incremental_mass = (incremental_mass / 3) - 2;
    if (incremental_mass > 0) {
      fuel_mass += incremental_mass;
    }
  }
  return fuel_mass;
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
