#include "World.h"
#include "CAPITAL_Engine.h"

World::World() {
  _log.console("{ (:) }", "constructing World");
}

World::~World() {}

VkVertexInputBindingDescription World::Cell::getBindingDescription() {
  VkVertexInputBindingDescription bindingDescription{};
  bindingDescription.binding = 0;
  bindingDescription.stride = sizeof(Cell);
  bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  return bindingDescription;
}

std::array<float, 6> World::setAliveCells() {
  std::array<float, 6> localArray = {1.0f};
  _log.console("testtesttest!!!!!!!!!!!!!!!!!!!", localArray[0]);
  return localArray;
}
