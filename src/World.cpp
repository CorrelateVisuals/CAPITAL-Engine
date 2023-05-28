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
      glm::lookAt(camera.position, camera.position + camera.front, camera.up);

  return view;
}

std::vector<World::Cell> World::initializeCells() {
  _log.console(_log.style.charLeader, "initializing Cells");
  static const std::array<float, 4> red = {1.0f, 0.0f, 0.0f, 1.0f};
  static const std::array<float, 4> blue = {0.0f, 0.0f, 1.0f, 1.0f};
  static const std::array<int, 4> alive = {1, 0, 0, 0};
  static const std::array<int, 4> dead = {-1, 0, 0, 0};

  // Grid size
  const int gridWidth = _control.grid.dimensions[0];
  const int gridHeight = _control.grid.dimensions[1];
  const float gridPointDistance = _control.grid.distance;
  const int numGridPoints = gridWidth * gridHeight;
  const int numAliveCells = _control.grid.numberOfAliveCells;

  if (numAliveCells > numGridPoints) {
    throw std::runtime_error(
        "Number of alive cells exceeds number of grid points");
  }

  std::vector<World::Cell> cells(numGridPoints);

  // Grid cell size
  const float cellWidth = gridPointDistance / gridWidth;
  const float cellHeight = gridPointDistance / gridHeight;
  // Cell position offset
  const float remainingWidth = 2.0f - gridPointDistance;
  const float remainingHeight = 2.0f - gridPointDistance;
  const float offsetX = -1.0f + remainingWidth / 2.0f + cellWidth / 2.0f;
  const float offsetY = -1.0f + remainingHeight / 2.0f + cellHeight / 2.0f;

  // Generate random alive cells
  std::vector<int> aliveCells =
      _control.setCellsAliveRandomly(_control.grid.numberOfAliveCells);

  // Initialize cells on grid
  std::vector<float> randomHeights(numGridPoints);
  std::vector<bool> isAliveIndices(numGridPoints, false);

  // Precompute random heights
  for (int i = 0; i < numGridPoints; i++) {
    randomHeights[i] = _control.getRandomFloat(0.0f, 0.3f);
  }

  // Precompute alive cell indices
  for (int aliveIndex : aliveCells) {
    isAliveIndices[aliveIndex] = true;
  }

  for (int i = 0; i < numGridPoints; i++) {
    int x = i % gridWidth;
    int y = i / gridWidth;
    float rowOffset = offsetX + x * cellWidth;
    float columnOffset = offsetY + y * cellHeight;
    float randomHeight = randomHeights[i];
    std::array<float, 4> pos = {rowOffset, columnOffset, randomHeight, 0.0f};

    bool isAlive = isAliveIndices[i];

    const std::array<float, 4>& color = isAlive ? blue : red;
    const std::array<int, 4>& state = isAlive ? alive : dead;

    cells[i] = {pos, color, cell.size, state};
  }
  //_log.console(aliveCells);
  return cells;
}

// Bug: The code is not checking if the number of alive cells is greater than
// the number of grid points. If it is, the code will try to

bool World::isIndexAlive(const std::vector<int>& aliveCells, int index) {
  return std::find(aliveCells.begin(), aliveCells.end(), index) !=
         aliveCells.end();
}

World::UniformBufferObject World::updateUniforms() {
  UniformBufferObject uniformObject{};
  uniformObject.dimensions = _control.grid.dimensions;
  uniformObject.passedHours = _control.passedSimulationHours;
  uniformObject.model = _world.setModel();
  uniformObject.view = _world.setView();
  uniformObject.proj = _world.setProjection(_mechanics.swapChain.extent);
  uniformObject.lightDirection = {0.0f, 1.0f, 1.0f, 0.2f};
  uniformObject.cellSize = cell.size[0];
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
      swapChainExtent.width / static_cast<float>(swapChainExtent.height), 0.1f,
      10.0f);

  projection[1][1] *= -1;  // flip y axis
  projection[0][0] *= -1;  // flip x axis

  return projection;
};
