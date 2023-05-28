#include "World.h"
#include "CapitalEngine.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>

World::World() {
  _log.console("{ (X) }", "constructing World");
}

World::~World() {
  _log.console("{ (X) }", "destructing World");
}

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
  static const std::array<float, 4> red = {1.0f, 0.0f, 0.0f, 1.0f};
  static const std::array<float, 4> blue = {0.0f, 0.0f, 1.0f, 1.0f};
  static const std::array<int, 4> alive = {1, 0, 0, 0};
  static const std::array<int, 4> dead = {-1, 0, 0, 0};

  const uint32_t width = _control.grid.dimensions[0];
  const uint32_t height = _control.grid.dimensions[1];
  const uint32_t numGridPoints = width * height;
  const uint32_t numAliveCells = _control.grid.totalAliveCells;
  const float gap = _control.grid.gap;

  if (numAliveCells > numGridPoints) {
    throw std::runtime_error(
        "!ERROR! Number of alive cells exceeds number of grid points");
  }

  std::vector<World::Cell> cells(numGridPoints);
  std::vector<float> randomHeights(numGridPoints, 0.0f);
  std::vector<bool> isAliveIndices(numGridPoints, false);

  for (uint32_t i = 0; i < numGridPoints; i++) {
    randomHeights.push_back(_control.getRandomFloat(0.0f, 0.3f));
    isAliveIndices.push_back(false);
  }

  std::vector<int> aliveCellIndices =
      _control.setCellsAliveRandomly(_control.grid.totalAliveCells);
  for (int aliveIndex : aliveCellIndices) {
    isAliveIndices[aliveIndex] = true;
  }

  const float startX = -((width - 1) * gap) / 2.0f;
  const float startY = -((height - 1) * gap) / 2.0f;

  for (uint32_t i = 0; i < numGridPoints; ++i) {
    const uint32_t x = i % width;
    const uint32_t y = i / width;
    const float posX = startX + x * gap;
    const float posY = startY + y * gap;

    const float randomHeight = randomHeights[i];
    const std::array<float, 4> pos = {posX, posY, randomHeight, 0.0f};
    const bool isAlive = isAliveIndices[i];

    const std::array<float, 4>& color = isAlive ? blue : red;
    const std::array<int, 4>& state = isAlive ? alive : dead;

    cells[i] = {pos, color, cell.size, state};
  }
  return cells;
}

bool World::isIndexAlive(const std::vector<int>& aliveCells, int index) {
  return std::find(aliveCells.begin(), aliveCells.end(), index) !=
         aliveCells.end();
}

World::UniformBufferObject World::updateUniforms() {
  UniformBufferObject uniformObject{};
  uniformObject.gridDimensions = _control.grid.dimensions;
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
