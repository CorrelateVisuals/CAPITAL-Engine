#include "World.h"
#include "CAPITAL_Engine.h"

#include <cstdlib>
#include <ctime>

World::World() {
  _log.console("{ (:) }", "constructing World");
}

World::~World() {}

std::vector<int> World::setCellsAliveRandomly(int size) {
  std::vector<int> IDs;
  srand(static_cast<unsigned int>(
      time(NULL)));  // seed the random number generator

  while (IDs.size() < size) {
    int ID = rand() % grid.numGridPoints;
    // check if the ID is not already in IDs
    if (std::find(IDs.begin(), IDs.end(), ID) == IDs.end()) {
      IDs.push_back(ID);
    }
  }
  return IDs;
}

VkVertexInputBindingDescription World::Cell::getBindingDescription() {
  VkVertexInputBindingDescription bindingDescription{};
  bindingDescription.binding = 0;
  bindingDescription.stride = sizeof(Cell);
  bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  return bindingDescription;
}
