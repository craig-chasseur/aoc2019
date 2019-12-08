#include <algorithm>
#include <deque>
#include <functional>
#include <fstream>
#include <iostream>
#include <limits>
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
    std::vector<int>* program, std::vector<int>::const_iterator pc,
    std::deque<int>* inputs) {
  CHECK(pc + 2 <= program->end());
  CHECK(!inputs->empty());
  const int value = inputs->front();
  inputs->pop_front();
  Store(value, *++pc, program);
  return ++pc;
}

std::vector<int>::const_iterator Output(
    std::vector<int>* program, std::vector<int>::const_iterator pc,
    std::deque<int>* outputs) {
  CHECK(pc + 2 <= program->end());
  // Fun fact: order of evaluation of function params is undefined behavior in
  // C++.
  const int mode = *pc / 100;
  const int value = LoadParam(mode, *++pc, *program);
  outputs->push_back(value);
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

class IntcodeMachine {
 public:
  enum class ExecState {
    kPendingInput,
    kHalt
  };

  struct RunResult {
    ExecState state;
    std::deque<int> outputs;
  };

  explicit IntcodeMachine(std::vector<int> program)
      : program_memory_(std::move(program)), pc_(program_memory_.begin()) {}

  RunResult Run() {
    std::deque<int> outputs;
    for (;;) {
      CHECK(pc_ >= program_memory_.begin() && pc_ < program_memory_.end());
      switch (*pc_ % 100) {
        case 1:
          pc_ = Add(&program_memory_, pc_);
          break;
        case 2:
          pc_ = Mul(&program_memory_, pc_);
          break;
        case 3:
          if (queued_inputs_.empty()) {
            return {ExecState::kPendingInput, std::move(outputs)};
          }
          pc_ = Input(&program_memory_, pc_, &queued_inputs_);
          break;
        case 4:
          pc_ = Output(&program_memory_, pc_, &outputs);
          break;
        case 5:
          pc_ = JumpIfTrue(&program_memory_, pc_);
          break;
        case 6:
          pc_ = JumpIfFalse(&program_memory_, pc_);
          break;
        case 7:
          pc_ = LessThan(&program_memory_, pc_);
          break;
        case 8:
          pc_ = Equals(&program_memory_, pc_);
          break;
        case 99:
          return {ExecState::kHalt, std::move(outputs)};
        default:
          CHECK(false);
      }
    }
  }

  void PushInputs(const std::deque<int>& inputs) {
    queued_inputs_.insert(queued_inputs_.end(), inputs.begin(), inputs.end());
  }

 private:
  std::vector<int> program_memory_;
  std::vector<int>::const_iterator pc_;
  std::deque<int> queued_inputs_;
};

int RunAmplifiers(const std::vector<int>& program,
                  const std::vector<int>& phase_settings) {
  std::vector<IntcodeMachine> amplifiers;
  for (const int phase : phase_settings) {
    amplifiers.emplace_back(program);
    amplifiers.back().PushInputs({phase});
  }
  auto amp_it = amplifiers.begin();
  std::deque<int> signals{0};
  for (;;) {
    amp_it->PushInputs(std::move(signals));
    IntcodeMachine::RunResult result = amp_it->Run();
    signals = std::move(result.outputs);
    if (result.state == IntcodeMachine::ExecState::kHalt &&
        amp_it + 1 == amplifiers.end()) {
      CHECK(!signals.empty());
      const int last_output = signals.front();
      signals.pop_front();
      CHECK(signals.empty());
      return last_output;
    }
    if (++amp_it == amplifiers.end()) {
      amp_it = amplifiers.begin();
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
  std::vector<int> phases{5, 6, 7, 8, 9};
  int max_out = std::numeric_limits<int>::min();
  do {
    max_out = std::max(max_out, RunAmplifiers(program, phases));
  } while (std::next_permutation(phases.begin(), phases.end()));
  std::cout << max_out << "\n";
  return 0;
}
