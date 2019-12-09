#ifndef CC_UTIL_INTCODE_H_
#define CC_UTIL_INTCODE_H_

#include <cstdint>
#include <deque>
#include <utility>
#include <vector>

namespace aoc2019 {

std::vector<std::int64_t> ReadIntcodeProgram(const char* filename);

class IntcodeMachine {
 public:
  enum class ExecState {
    kPendingInput,
    kHalt
  };

  struct RunResult {
    ExecState state;
    std::deque<std::int64_t> outputs;
  };

  explicit IntcodeMachine(std::vector<std::int64_t> program)
      : program_memory_(std::move(program)) {}

  RunResult Run();

  void RunWithConsoleIO();

  void PushInputs(const std::deque<std::int64_t>& inputs);

 private:
  enum class AddressingMode {
    kAbsolute = 0,
    kImmediate = 1,
    kRelative = 2
  };

  static AddressingMode GetAddressingMode(std::int64_t mode_field);

  void MaybeGrow(std::vector<std::int64_t>::size_type position);

  std::int64_t LoadParam(std::int64_t mode, std::int64_t value);

  void Store(std::int64_t mode, std::int64_t value, std::int64_t position);

  template <typename Op>
  void Math3();

  void Add();
  void Mul();

  // Returns true if input was consumed, false if needs more input.
  bool Input();
  void Output(std::deque<std::int64_t>* outputs);

  template <bool if_true>
  void ConditionalJump();

  void JumpIfTrue();
  void JumpIfFalse();

  void LessThan();
  void Equals();

  void AdjustRelativeBase();

  std::vector<std::int64_t> program_memory_;
  std::vector<std::int64_t>::size_type pc_ = 0;
  std::deque<std::int64_t> queued_inputs_;
  std::int64_t relative_base_ = 0;
};

}  // namespace aoc2019

#endif  // CC_UTIL_INTCODE_H_
