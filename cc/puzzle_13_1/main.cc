#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "cc/util/check.h"
#include "cc/util/intcode.h"

namespace {

class ArcadeMachine {
 public:
  explicit ArcadeMachine(std::vector<std::int64_t> program)
      : cpu_(std::move(program)) {}

  void Run() {
    aoc2019::IntcodeMachine::RunResult result;
    do {
      result = cpu_.Run();

      CHECK(result.outputs.size() % 3 == 0);
      while (!result.outputs.empty()) {
        const std::int64_t x = result.outputs.front();
        result.outputs.pop_front();
        const std::int64_t y = result.outputs.front();
        result.outputs.pop_front();
        const std::int64_t tile = result.outputs.front();
        result.outputs.pop_front();
        if (x == -1 && y == 0) {
          score_ = tile;
        } else {
          display_.DrawTile(x, y, tile);
        }
      }
      display_.Render();
      std::cout << "\nSCORE: " << score_ << "\n";

      while (result.state ==
             aoc2019::IntcodeMachine::ExecState::kPendingInput) {
        char input;
        std::cin >> input;
        switch (input) {
          case 'a':
            cpu_.PushInputs({-1});
            break;
          case 's':
            cpu_.PushInputs({0});
            break;
          case 'd':
            cpu_.PushInputs({1});
            break;
          default:
            continue;
        }
        break;
      }
    } while (result.state != aoc2019::IntcodeMachine::ExecState::kHalt);
  }

 private:
  class Display {
   public:
    Display() = default;

    void DrawTile(std::int64_t x, std::int64_t y, std::int64_t tile) {
      MaybeGrowGrid(x, y);
      switch (tile) {
        case 0:
          grid_[y][x] = ' ';
          break;
        case 1:
          grid_[y][x] = '|';
          break;
        case 2:
          grid_[y][x] = '#';
          break;
        case 3:
          grid_[y][x] = '_';
          break;
        case 4:
          grid_[y][x] = '*';
          break;
        default:
          std::cerr << "Invalid tile: " << tile << "\n";
          CHECK(false);
      }
    }

    void Render() const {
      for (const std::string& line : grid_) {
        std::cout << line << "\n";
      }
    }

   private:
    void MaybeGrowGrid(std::int64_t x, std::int64_t y) {
      if (grid_.empty()) {
        grid_.emplace_back(x + 1, ' ');
      }
      if (y >= grid_.size()) {
        grid_.resize(y + 1, grid_.front());
      }
      if (x >= grid_.front().size()) {
        for (std::string& line : grid_) {
          line.resize(x + 1, ' ');
        }
      }
    }

    std::vector<std::string> grid_;
  };

  aoc2019::IntcodeMachine cpu_;
  std::int64_t score_ = 0;
  Display display_;
};

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  std::vector<std::int64_t> program = aoc2019::ReadIntcodeProgram(argv[1]);
  program[0] = 2;
  ArcadeMachine machine(std::move(program));
  machine.Run();
  return 0;
}
