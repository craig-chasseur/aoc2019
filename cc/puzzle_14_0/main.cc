#include <cstdio>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "cc/util/check.h"

namespace {

class Reaction {
 public:
  struct Info {
    absl::flat_hash_map<std::string, int> needed_reagents;
    int surplus_product = 0;
  };

  explicit Reaction(absl::string_view description) {
    auto c_str_buffer = std::make_unique<char[]>(description.size() + 1);

    std::vector<absl::string_view> inputs = absl::StrSplit(description, " => ");
    CHECK(inputs.size() == 2);

    CHECK(2 == std::sscanf(inputs.back().data(), "%d %s",
                           &product_volume_, c_str_buffer.get()));
    product_ = std::string(c_str_buffer.get());

    for (absl::string_view reagent_desc :
         absl::StrSplit(inputs.front(), ", ")) {
      int quantity;
      CHECK(2 == std::sscanf(reagent_desc.data(), "%d %s",
                             &quantity, c_str_buffer.get()));
      std::string reagent(c_str_buffer.get());
      if (reagent.back() == ',') reagent.pop_back();
      CHECK(reagents_.try_emplace(std::move(reagent), quantity).second);
    }
  }

  const std::string& product() const {
    return product_;
  }

  Info Reverse(int needed_product) const {
    int batches = (needed_product + product_volume_ - 1) / product_volume_;
    CHECK(batches > 0);
    Info info;
    info.needed_reagents = reagents_;
    for (auto& reagent : info.needed_reagents) {
      reagent.second *= batches;
    }
    info.surplus_product = (batches * product_volume_) - needed_product;
    return info;
  }

 private:
  absl::flat_hash_map<std::string, int> reagents_;
  std::string product_;
  int product_volume_ = 0;
};

class NanoFactory {
 public:
  NanoFactory() = default;

  void AddReaction(absl::string_view description) {
    Reaction reaction(description);
    std::string product = reaction.product();
    CHECK(product_to_reaction_.try_emplace(
        std::move(product), std::move(reaction)).second);
  }

  int CalculateOreForFuel(int needed_fuel) const {
    absl::flat_hash_map<std::string, int> needed_reagents;
    needed_reagents.emplace("FUEL", needed_fuel);
    int needed_ore = 0;
    absl::flat_hash_map<std::string, int> surplus_reagents;
    while (!needed_reagents.empty()) {
      auto needed_it = needed_reagents.begin();

      // Apply any leftovers we have on hand.
      auto surplus_it = surplus_reagents.find(needed_it->first);
      if (surplus_it != surplus_reagents.end()) {
        needed_it->second -= surplus_it->second;
        if (needed_it->second < 0) {
          // We had more than we needed.
          surplus_it->second = -needed_it->second;
          needed_reagents.erase(needed_it);
          continue;
        }
        if (needed_it->second == 0) {
          // We had exactly as much as we needed.
          needed_reagents.erase(needed_it);
          surplus_reagents.erase(surplus_it);
          continue;
        }
        // We had some of what we needed, but not enough.
        surplus_reagents.erase(surplus_it);
      }

      auto reaction_it = product_to_reaction_.find(needed_it->first);
      CHECK(reaction_it != product_to_reaction_.end());
      Reaction::Info info = reaction_it->second.Reverse(needed_it->second);
      if (info.surplus_product > 0) {
        surplus_reagents[needed_it->first] += info.surplus_product;
      }
      needed_reagents.erase(needed_it);

      for (const auto& new_reagent : info.needed_reagents) {
        if (new_reagent.first == "ORE") {
          needed_ore += new_reagent.second;
          continue;
        }
        needed_reagents[new_reagent.first] += new_reagent.second;
      }
    }

    return needed_ore;
  }

 private:
  absl::flat_hash_map<std::string, Reaction> product_to_reaction_;
};

NanoFactory LoadReactions(const char* filename) {
  std::ifstream stream(filename);
  CHECK(stream);
  NanoFactory nano_factory;
  std::string line;
  while (std::getline(stream, line)) {
    nano_factory.AddReaction(line);
  }
  stream.close();
  return nano_factory;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  NanoFactory nano_factory = LoadReactions(argv[1]);
  std::cout << nano_factory.CalculateOreForFuel(1) << "\n";
  return 0;
}
