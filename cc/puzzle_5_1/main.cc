#include <functional>
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

int LoadParam(int mode, int value, const std::vector<int>& program) {
  if ((mode & 0x1) != 0) return value;
  CHECK(value < program.size());
  return program[value];
}

void Store(int value, int position, std::vector<int>* program) {
  CHECK(position < program->size());
  (*program)[position] = value;
}

template <typename Op>
std::vector<int>::const_iterator Math3(
    std::vector<int>* program, std::vector<int>::const_iterator pc) {
  CHECK(pc + 4 <= program->end());
  int opcode = *pc;
  Store(Op()(LoadParam(opcode / 100, *++pc, *program),
             LoadParam(opcode / 1000, *++pc, *program)),
        *++pc, program);
  return ++pc;
}

std::vector<int>::const_iterator Add(
    std::vector<int>* program, std::vector<int>::const_iterator pc) {
  return Math3<std::plus<int>>(program, pc);
}

std::vector<int>::const_iterator Mul(
    std::vector<int>* program, std::vector<int>::const_iterator pc) {
  return Math3<std::multiplies<int>>(program, pc);
}

std::vector<int>::const_iterator Input(
    std::vector<int>* program, std::vector<int>::const_iterator pc) {
  CHECK(pc + 2 <= program->end());
  std::cout << ">> ";
  int value;
  std::cin >> value;
  Store(value, *++pc, program);
  return ++pc;
}

std::vector<int>::const_iterator Output(
    std::vector<int>* program, std::vector<int>::const_iterator pc) {
  CHECK(pc + 2 <= program->end());
  // Fun fact: order of evaluation of function params is undefined behavior in
  // C++.
  const int mode = *pc / 100;
  const int value = LoadParam(mode, *++pc, *program);
  std::cout << value << "\n";
  return ++pc;
}

template <bool if_true>
std::vector<int>::const_iterator ConditionalJump(
    std::vector<int>* program, std::vector<int>::const_iterator pc) {
  CHECK(pc + 3 <= program->end());
  const int opcode = *pc;
  const int value = LoadParam(opcode / 100, *++pc, *program);
  if constexpr (if_true) {
    if (value == 0) return pc + 2;
  } else {
    if (value != 0) return pc + 2;
  }
  return program->begin() + LoadParam(opcode / 1000, *++pc, *program);
}

std::vector<int>::const_iterator JumpIfTrue(
    std::vector<int>* program, std::vector<int>::const_iterator pc) {
  return ConditionalJump<true>(program, pc);
}

std::vector<int>::const_iterator JumpIfFalse(
    std::vector<int>* program, std::vector<int>::const_iterator pc) {
  return ConditionalJump<false>(program, pc);
}

std::vector<int>::const_iterator LessThan(
    std::vector<int>* program, std::vector<int>::const_iterator pc) {
  return Math3<std::less<int>>(program, pc);
}

std::vector<int>::const_iterator Equals(
    std::vector<int>* program, std::vector<int>::const_iterator pc) {
  return Math3<std::equal_to<int>>(program, pc);
}

void RunProgram(std::vector<int>* program) {
  std::vector<int>::const_iterator pc = program->begin();
  for (;;) {
    CHECK(pc >= program->begin() && pc < program->end());
    switch (*pc % 100) {
      case 1:
        pc = Add(program, pc);
        break;
      case 2:
        pc = Mul(program, pc);
        break;
      case 3:
        pc = Input(program, pc);
        break;
      case 4:
        pc = Output(program, pc);
        break;
      case 5:
        pc = JumpIfTrue(program, pc);
        break;
      case 6:
        pc = JumpIfFalse(program, pc);
        break;
      case 7:
        pc = LessThan(program, pc);
        break;
      case 8:
        pc = Equals(program, pc);
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
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  std::vector<int> program = ReadProgram(argv[1]);
  RunProgram(&program);
  return 0;
}
