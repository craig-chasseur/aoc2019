#include "cc/util/intcode.h"

#include <cstdint>
#include <deque>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/numbers.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "cc/util/check.h"

namespace aoc2019 {

std::vector<std::int64_t> ReadIntcodeProgram(const char* filename) {
  std::ifstream stream(filename);
  CHECK(stream);
  std::string buffer;
  stream.seekg(0, std::ios::end);
  buffer.resize(stream.tellg());
  stream.seekg(0, std::ios::beg);
  stream.read(&buffer[0], buffer.size());
  stream.close();

  std::vector<std::int64_t> program;
  for (const absl::string_view str :
       absl::StrSplit(buffer, absl::ByChar(','))) {
    std::int64_t element;
    CHECK(absl::SimpleAtoi(str, &element));
    program.push_back(element);
  }
  return program;
}

IntcodeMachine::RunResult IntcodeMachine::Run() {
  std::deque<std::int64_t> outputs;
  for (;;) {
    MaybeGrow(pc_);
    switch (program_memory_[pc_] % 100) {
      case 1:
        Add();
        break;
      case 2:
        Mul();
        break;
      case 3:
        if (!Input()) {
          return {ExecState::kPendingInput, std::move(outputs)};
        }
        break;
      case 4:
        Output(&outputs);
        break;
      case 5:
        JumpIfTrue();
        break;
      case 6:
        JumpIfFalse();
        break;
      case 7:
        LessThan();
        break;
      case 8:
        Equals();
        break;
      case 9:
        AdjustRelativeBase();
        break;
      case 99:
        return {ExecState::kHalt, std::move(outputs)};
      default:
        std::cerr << "Unrecognized opcode: " << program_memory_[pc_] << "\n";
        CHECK(false);
    }
  }
}

void IntcodeMachine::RunWithConsoleIO() {
  RunResult result;
  do {
    result = Run();
    std::cout << absl::StrJoin(result.outputs, ",") << "\n";
    if (result.state == ExecState::kPendingInput) {
      std::cout << "INPUT> ";
      std::int64_t input;
      std::cin >> input;
      PushInputs({input});
    }
  } while (result.state != ExecState::kHalt);
}

void IntcodeMachine::RunWithAsciiConsoleIO() {
  RunResult result;
  do {
    result = Run();
    for (const std::int64_t val : result.outputs) {
      if (val >= 0 && val < 128) {
        std::cout << static_cast<char>(val);
      } else {
        std::cout << "Non printable: " << val << "\n";
      }
    }
    std::cout.flush();
    if (result.state == ExecState::kPendingInput) {
      std::string inputstr;
      std::getline(std::cin, inputstr);
      if (inputstr.back() != '\n') inputstr.push_back('\n');
      PushInputs(std::deque<std::int64_t>(inputstr.begin(), inputstr.end()));
    }
  } while (result.state != ExecState::kHalt);
}

void IntcodeMachine::PushInputs(const std::deque<std::int64_t>& inputs) {
  queued_inputs_.insert(queued_inputs_.end(), inputs.begin(), inputs.end());
}

IntcodeMachine::AddressingMode IntcodeMachine::GetAddressingMode(
    std::int64_t mode_field) {
  switch (mode_field % 10) {
    case 0:
      return AddressingMode::kAbsolute;
    case 1:
      return AddressingMode::kImmediate;
    case 2:
      return AddressingMode::kRelative;
    default:
      std::cerr << "Invalid addressing mode: " << (mode_field % 10) << "\n";
      CHECK(false);
  }
}

void IntcodeMachine::MaybeGrow(std::vector<std::int64_t>::size_type position) {
  if (position < program_memory_.size()) return;
  program_memory_.resize(position + 1, 0);
}

std::int64_t IntcodeMachine::LoadParam(std::int64_t mode, std::int64_t value) {
  switch (GetAddressingMode(mode)) {
    case AddressingMode::kAbsolute:
      MaybeGrow(value);
      return program_memory_[value];
    case AddressingMode::kImmediate:
      return value;
    case AddressingMode::kRelative: {
      const std::vector<std::int64_t>::size_type position =
          relative_base_ + value;
      MaybeGrow(position);
      return program_memory_[position];
    }
  }
  CHECK(false);
}

void IntcodeMachine::Store(std::int64_t mode, std::int64_t value,
                           std::int64_t position) {
  switch (GetAddressingMode(mode)) {
    case AddressingMode::kAbsolute:
      MaybeGrow(position);
      program_memory_[position] = value;
      return;
    case AddressingMode::kImmediate:
      std::cerr << "Can't store with immediate mode destination\n";
      CHECK(false);
    case AddressingMode::kRelative:
      position += relative_base_;
      MaybeGrow(position);
      program_memory_[position] = value;
      return;
  }
}

template <typename Op>
void IntcodeMachine::Math3() {
  MaybeGrow(pc_ + 3);
  const std::int64_t opcode = program_memory_[pc_++];
  const std::int64_t param0 = LoadParam(opcode / 100, program_memory_[pc_++]);
  const std::int64_t param1 = LoadParam(opcode / 1000,
                                        program_memory_[pc_++]);
  const std::int64_t result = Op()(param0, param1);
  Store(opcode / 10000, result, program_memory_[pc_++]);
}

void IntcodeMachine::Add() {
  Math3<std::plus<std::int64_t>>();
}

void IntcodeMachine::Mul() {
  Math3<std::multiplies<std::int64_t>>();
}

bool IntcodeMachine::Input() {
  if (queued_inputs_.empty()) return false;
  MaybeGrow(pc_ + 1);
  const std::int64_t value = queued_inputs_.front();
  queued_inputs_.pop_front();
  const std::int64_t mode = program_memory_[pc_++] / 100;
  Store(mode, value, program_memory_[pc_++]);
  return true;
}

void IntcodeMachine::Output(std::deque<std::int64_t>* outputs) {
  MaybeGrow(pc_ + 1);
  const std::int64_t mode = program_memory_[pc_++] / 100;
  const std::int64_t value = LoadParam(mode, program_memory_[pc_++]);
  outputs->push_back(value);
}

template <bool if_true>
void IntcodeMachine::ConditionalJump() {
  MaybeGrow(pc_ + 2);
  const std::int64_t opcode = program_memory_[pc_++];
  const std::int64_t value = LoadParam(opcode / 100, program_memory_[pc_++]);
  if constexpr (if_true) {
    if (value == 0) {
      ++pc_;
      return;
    }
  } else {
    if (value != 0) {
      ++pc_;
      return;
    }
  }
  pc_ = LoadParam(opcode / 1000, program_memory_[pc_]);
}

void IntcodeMachine::JumpIfTrue() {
  ConditionalJump<true>();
}

void IntcodeMachine::JumpIfFalse() {
  ConditionalJump<false>();
}

void IntcodeMachine::LessThan() {
  Math3<std::less<std::int64_t>>();
}

void IntcodeMachine::Equals() {
  Math3<std::equal_to<std::int64_t>>();
}

void IntcodeMachine::AdjustRelativeBase() {
  MaybeGrow(pc_ + 1);
  const std::int64_t mode = program_memory_[pc_++] / 100;
  relative_base_ += LoadParam(mode, program_memory_[pc_++]);
}

}  // namespace aoc2019
