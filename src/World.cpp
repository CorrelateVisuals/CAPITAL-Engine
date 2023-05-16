#include "World.h"
#include "CapitalEngine.h"

#include <algorithm>
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
  return attributeDescriptions;
}

glm::mat4 World::setView() {
  glm::mat4 view =
      glm::lookAt(glm::vec3(0.0f, 0.0f, 3.5f), glm::vec3(0.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, -1.0f, 0.0f));
  return view;
}

std::vector<World::Cell> World::initializeCells() {
  _log.console(_log.style.charLeader, "initializing Cells");
  std::vector<World::Cell> cells(_control.grid.numberOfGridPoints);

  std::array<float, 4> red = {1.0, 0.0, 0.0, 1.0};
  std::array<float, 4> blue = {0.0, 0.0, 1.0, 1.0};
  std::array<float, 4> alive = {0.0, 0.0, 0.0, 1.0};
  std::array<float, 4> dead = {0.0, 0.0, 0.0, -1.0};
  std::array<float, 4> size = {_control.grid.cellSize, _control.grid.cellSize,
                               _control.grid.cellSize, _control.grid.cellSize};
  std::vector<int> aliveCells =
      _control.setCellsAliveRandomly(_control.grid.numberOfAliveCells);

  // Grid size
  const int gridWidth = _control.grid.gridDimensions[0];
  const int gridHeight = _control.grid.gridDimensions[1];
  const float gridPointDistance = _control.grid.gridPointDistance;
  // Grid cell size
  const float cellWidth = gridPointDistance / gridWidth;
  const float cellHeight = gridPointDistance / gridHeight;
  // Cell position offset
  const float remainingWidth = 2.0f - gridPointDistance;
  const float remainingHeight = 2.0f - gridPointDistance;
  const float offsetX = -1.0f + remainingWidth / 2.0f + cellWidth / 2.0f;
  const float offsetY = -1.0f + remainingHeight / 2.0f + cellHeight / 2.0f;

  // Initialize cells on grid
  for (int x = 0; x < gridWidth; x++) {
    for (int y = 0; y < gridHeight; y++) {
      int index = x + y * gridWidth;
      std::array<float, 4> pos = {offsetX + x * cellWidth,
                                  offsetY + y * cellHeight,
                                  _control.getRandomFloat(0.0f, 0.3f), 1.0f};

      World::Cell initializedCell{pos, blue, size, alive};
      World::Cell uninitializedCell{pos, red, size, dead};

      if (std::find(aliveCells.begin(), aliveCells.end(), index) !=
          aliveCells.end()) {
        cells[index] = initializedCell;
      } else {
        cells[index] = uninitializedCell;
      }
    }
  }
  _log.console(aliveCells);

  return cells;
}

World::UniformBufferObject World::updateUniforms() {
  UniformBufferObject uniformObject{};
  uniformObject.passedHours = _control.passedSimulationHours;
  uniformObject.model = _world.setModel();
  uniformObject.view = _world.setView();
  uniformObject.proj = _world.setProjection(_mechanics.swapChain.extent);
  uniformObject.lightDirection = {0.0f, 1.0f, 1.0f, 0.2f};
  return uniformObject;
}
glm::mat4 World::setModel() {
  glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f),
                                glm::vec3(0.0f, 0.0f, 1.0f));
  return model;
}

glm::mat4 World::setProjection(VkExtent2D& swapChainExtent) {
  glm::mat4 projection = glm::perspective(
      glm::radians(60.0f),
      swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);

  projection[1][1] *= -1;  // flip y axis
  projection[0][0] *= -1;  // flip x axis

  return projection;
};
