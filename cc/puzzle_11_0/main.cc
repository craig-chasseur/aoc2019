#include <cstddef>
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "absl/hash/hash.h"
#include "cc/util/check.h"
#include "cc/util/intcode.h"

namespace {

class PainterBot {
 public:
  explicit PainterBot(std::vector<std::int64_t> program)
      : brain_(std::move(program)) {}

  std::size_t RunAndCountPositions() {
    absl::flat_hash_set<Position> painted;
    absl::flat_hash_set<Position> white;
    aoc2019::IntcodeMachine::RunResult result;
    do {
      result = brain_.Run();
      if (!result.outputs.empty()) {
        CHECK(result.outputs.size() == 2);

        switch (result.outputs.front()) {
          case 0:
            painted.insert(position_);
            white.erase(position_);
            break;
          case 1:
            painted.insert(position_);
            white.insert(position_);
            break;
          default:
            std::cerr << "Invalid paint command: " << result.outputs.front();
            CHECK(false);
        }

        Move(result.outputs.back());
      }

      if (result.state == aoc2019::IntcodeMachine::ExecState::kPendingInput) {
        brain_.PushInputs({white.contains(position_) ? 1 : 0});
      }
    } while (result.state != aoc2019::IntcodeMachine::ExecState::kHalt);

    return painted.size();
  }

 private:
  enum class Facing : std::int8_t {
    kUp = 0,
    kRight = 1,
    kDown = 2,
    kLeft = 3
  };

  struct Position {
    int x = 0;
    int y = 0;

    bool operator==(const Position& other) const {
      return x == other.x && y == other.y;
    }

    template <typename H>
    friend H AbslHashValue(H h, const Position& pos) {
      return H::combine(std::move(h), pos.x, pos.y);
    }
  };

  static Facing Left(Facing orig) {
    return orig ==
        Facing::kUp ? Facing::kLeft
                    : static_cast<Facing>((static_cast<std::int8_t>(orig) - 1));
  }

  static Facing Right(Facing orig) {
    return static_cast<Facing>((static_cast<std::int8_t>(orig) + 1) % 4);
  }

  void Move(std::int64_t turn) {
    switch (turn) {
      case 0:
        facing_ = Left(facing_);
        break;
      case 1:
        facing_ = Right(facing_);
        break;
      default:
        std::cerr << "Invalid turn direction: " << turn;
        CHECK(false);
    }
    switch (facing_) {
      case Facing::kUp:
        --position_.y;
        break;
      case Facing::kRight:
        ++position_.x;
        break;
      case Facing::kDown:
        ++position_.y;
        break;
      case Facing::kLeft:
        --position_.x;
        break;
      default:
        CHECK(false);
    }
  }

  aoc2019::IntcodeMachine brain_;
  Facing facing_ = Facing::kUp;
  Position position_;
};

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  PainterBot bot(aoc2019::ReadIntcodeProgram(argv[1]));
  std::cout << bot.RunAndCountPositions() << "\n";
  return 0;
}
