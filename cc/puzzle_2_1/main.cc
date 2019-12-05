#include <cassert>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <vector>

#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "cc/util/check.h"

namespace {

std::vector<int> ReadProgram(const char* filename) {
  std::ifstream stream(filename);
  CHECK(stream);
  std::string buffer;
  stream.seekg(0, std::ios::end);
  buffer.resize(stream.tellg());
  stream.seekg(0, std::ios::beg);
  stream.read(&buffer[0], buffer.size());
  stream.close();

  std::vector<int> program;
  for (const absl::string_view str : absl::StrSplit(buffer, ",")) {
    int element;
    CHECK(absl::SimpleAtoi(str, &element));
    program.push_back(element);
  }
  return program;
}

void RangeCheck(const std::vector<int>& program, std::size_t pos) {
  CHECK(pos + 4 <= program.size());
  CHECK(program[pos + 1] < program.size());
  CHECK(program[pos + 2] < program.size());
  CHECK(program[pos + 3] < program.size());
}

void RunProgram(std::vector<int>* program) {
  std::size_t pos = 0;
  for (;;) {
    CHECK(pos < program->size());
    switch ((*program)[pos]) {
      case 1:
        RangeCheck(*program, pos);
        (*program)[(*program)[pos + 3]] = (*program)[(*program)[pos + 1]] +
                                          (*program)[(*program)[pos + 2]];
        pos += 4;
        break;
      case 2:
        RangeCheck(*program, pos);
        (*program)[(*program)[pos + 3]] = (*program)[(*program)[pos + 1]] *
                                          (*program)[(*program)[pos + 2]];
        pos += 4;
        break;
      case 99:
        return;
      default:
        CHECK(false);
    }
  }
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "USAGE: main FILENAME TARGET_OUTPUT\n";
    return 1;
  }
  const std::vector<int> program = ReadProgram(argv[1]);
  int target;
  CHECK(absl::SimpleAtoi(argv[2], &target));

  for (int noun = 0; noun < 100; ++noun) {
    for (int verb = 0; verb < 100; ++verb) {
      std::vector<int> program_copy(program);
      program_copy[1] = noun;
      program_copy[2] = verb;
      RunProgram(&program_copy);
      if (program_copy[0] == target) {
        std::cout << (100 * noun + verb) << "\n";
        return 0;
      }
    }
  }
  std::cerr << "Search failed\n";
  return 1;
}
