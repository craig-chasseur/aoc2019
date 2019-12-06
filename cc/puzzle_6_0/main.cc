#include <fstream>
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_split.h"
#include "cc/util/check.h"

namespace {

class OrbitGraph {
 public:
  OrbitGraph() = default;

  void AddOrbit(std::string orbiter, std::string center) {
    CHECK(name_to_node_.try_emplace(std::move(orbiter), std::move(center))
          .second);
  }

  int CountOrbits() {
    int total = 0;
    for (auto& name_and_node : name_to_node_) {
      total += CountOrbitsForNode(&name_and_node.second);
    }
    return total;
  }

 private:
  struct Node {
    explicit Node(std::string parent_in) : parent(parent_in) {}

    std::string parent;
    int orbit_count = -1;  // memoized once initially counted
  };

  int CountOrbitsForNode(Node* node) {
    if (node->orbit_count >= 0) return node->orbit_count;
    if (node->parent == "COM") {
      node->orbit_count = 1;
    } else {
      auto parent_it = name_to_node_.find(node->parent);
      CHECK(parent_it != name_to_node_.end());
      node->orbit_count = 1 + CountOrbitsForNode(&parent_it->second);
    }
    return node->orbit_count;
  }

  absl::flat_hash_map<std::string, Node> name_to_node_;
};

OrbitGraph ReadOrbits(const char* filename) {
  OrbitGraph graph;
  std::ifstream stream(filename);
  std::string buffer;
  while (stream >> buffer) {
    std::vector<std::string> parts = absl::StrSplit(buffer, absl::ByChar(')'));
    CHECK(parts.size() == 2);
    graph.AddOrbit(std::move(parts[1]), std::move(parts[0]));
  }
  stream.close();
  return graph;
}

}  // namespace

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USAGE: main FILENAME\n";
    return 1;
  }
  OrbitGraph graph = ReadOrbits(argv[1]);
  std::cout << graph.CountOrbits() << "\n";
  return 0;
}
