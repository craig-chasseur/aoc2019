#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "cc/util/check.h"

namespace {

constexpr std::size_t kImageSize = 25 * 6;

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

std::string MergeLayers(absl::string_view top, absl::string_view bottom) {
  CHECK(top.size() == bottom.size());
  std::string output(top.size(), '2');
  for (std::size_t pos = 0; pos < top.size(); ++pos) {
    output[pos] = top[pos] == '2' ? bottom[pos] : top[pos];
  }
  return output;
}

std::vector<absl::string_view> SplitLayers(std::string img) {
  std::vector<absl::string_view> layers;
  for (std::size_t start = 0; start < img.size() - 1; start += kImageSize) {
    layers.emplace_back(img.data() + start, kImageSize);
  }
  return layers;
}

void RenderLayer(absl::string_view layer) {
  for (int y = 0; y < 6; ++y) {
    for (int x = 0; x < 25; ++x) {
      char pixel = layer[y * 25 + x];
      std::cout << ((pixel == '0') ? ' ' : '+');
    }
    std::cout << "\n";
  }
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  const std::string img = ReadImg(argv[1]);
  CHECK(img.size() % kImageSize == 1);
  std::vector<absl::string_view> layers = SplitLayers(img);
  std::string output = std::string(layers.front());
  for (auto bottom_it = layers.begin() + 1; bottom_it != layers.end();
       ++bottom_it) {
    output = MergeLayers(output, *bottom_it);
  }
  RenderLayer(output);
  return 0;
}
