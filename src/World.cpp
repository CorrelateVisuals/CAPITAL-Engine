#include "World.h"
#include "CAPITAL_Engine.h"

#include <cstdlib>
#include <ctime>

World::World() {
  _log.console("{ (:) }", "constructing World");
}

World::~World() {}

std::vector<VkVertexInputBindingDescription> World::getBindingDescriptions() {
  std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
  bindingDescriptions.push_back(
      {0, sizeof(Cell), VK_VERTEX_INPUT_RATE_INSTANCE});
  // bindingDescriptions.push_back(
  //     {1, sizeof(Cube), VK_VERTEX_INPUT_RATE_INSTANCE});
  return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription>
World::getAttributeDescriptions() {
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
  attributeDescriptions.push_back(
      {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Cell, position)});
  attributeDescriptions.push_back(
      {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Cell, color)});
  attributeDescriptions.push_back(
      {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Cell, size)});
  attributeDescriptions.push_back(
      {3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Cell, gridSize)});
  attributeDescriptions.push_back(
      {4, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Cell, printGLSL)});
  return attributeDescriptions;
}

std::vector<int> World::setCellsAliveRandomly(int size) {
  std::vector<int> CellIDs;
  srand(static_cast<unsigned int>(
      time(NULL)));  // seed the random number generator

  while (CellIDs.size() < size) {
    int CellID = rand() % grid.numGridPoints;
    // check if the CellID is not already in CellIDs
    if (std::find(CellIDs.begin(), CellIDs.end(), CellID) == CellIDs.end()) {
      CellIDs.push_back(CellID);
    }
  }
  _log.console("  .....  ", "Alive cells:");
  for (auto& cellID : CellIDs) {
    std::cout << cellID << " ";
  }
  std::cout << "\n";
  return CellIDs;
}
