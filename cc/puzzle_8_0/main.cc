#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

#include "cc/util/check.h"

namespace {

std::string ReadImg(const char* filename) {
  std::ifstream stream(filename);
  CHECK(stream);
  std::string buffer;
  stream.seekg(0, std::ios::end);
  buffer.resize(stream.tellg());
  stream.seekg(0, std::ios::beg);
  stream.read(&buffer[0], buffer.size());
  stream.close();
  return buffer;
}

}  // namespace

int main(int argc, char** argv) {
  static constexpr std::size_t kImageSize = 25 * 6;
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  const std::string img = ReadImg(argv[1]);
  CHECK(img.size() % kImageSize == 1);

  int minzeroes = std::numeric_limits<int>::max();
  int maxmul = 0;
  for (std::size_t start = 0; start < img.size() - 1; start += kImageSize) {
    int zeroes =
        std::count(img.begin() + start, img.begin() + start + kImageSize, '0');
    if (zeroes < minzeroes) {
      minzeroes = zeroes;
      maxmul =
          std::count(img.begin() + start, img.begin() + start + kImageSize,
                     '1') *
          std::count(img.begin() + start, img.begin() + start + kImageSize,
                     '2');
    }
  }
  std::cout << maxmul << "\n";
}
