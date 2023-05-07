#include "World.h"
#include "CAPITAL_Engine.h"

#include <cstdlib>
#include <ctime>

World::World() {
  _log.console("{ (:) }", "constructing World");
}

World::~World() {}

int* World::setAliveCellsRnd(int size) {
  // Allocate memory for the array
  int* IDs = new int[size];
  // Seed the random number generator
  std::srand(static_cast<uint32_t>(std::time(nullptr)));

  for (int i = 0; i < size; ++i) {
    // Generate a random ID between 0 and grid.numGridPoints
    int ID = (std::rand() % grid.numGridPoints);
    IDs[i] = ID;
    _log.console("  .....  ", i, "Randomly set CellID Alive:", ID);
  }
  return IDs;
};

VkVertexInputBindingDescription World::Cell::getBindingDescription() {
  VkVertexInputBindingDescription bindingDescription{};
  bindingDescription.binding = 0;
  bindingDescription.stride = sizeof(Cell);
  bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  return bindingDescription;
}
