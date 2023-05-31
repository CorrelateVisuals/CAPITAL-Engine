#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "CapitalEngine.h"
#include "World.h"

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

std::vector<World::Cell> World::initializeCells() {
  const uint_fast16_t width = _control.grid.dimensions[0];
  const uint_fast16_t height = _control.grid.dimensions[1];
  const uint_fast32_t numGridPoints = width * height;
  const uint_fast32_t numAliveCells = _control.grid.totalAliveCells;
  const float gap = _control.grid.gap;
  std::array<float, 4> size = {_control.cells.size};

  if (numAliveCells > numGridPoints) {
    throw std::runtime_error(
        "!ERROR! Number of alive cells exceeds number of grid points");
  }

  std::vector<World::Cell> cells(numGridPoints);
  std::vector<bool> isAliveIndices(numGridPoints, false);

  for (uint_fast32_t i = 0; i < numGridPoints; i++) {
    isAliveIndices.push_back(false);
  }

  std::vector<uint_fast32_t> aliveCellIndices =
      _control.setCellsAliveRandomly(_control.grid.totalAliveCells);
  for (int aliveIndex : aliveCellIndices) {
    isAliveIndices[aliveIndex] = true;
  }

  float startX = -((width - 1) * gap) / 2.0f;
  float startY = -((height - 1) * gap) / 2.0f;

  for (uint_fast32_t i = 0; i < numGridPoints; ++i) {
    const uint_fast16_t x = static_cast<uint_fast16_t>(i % width);
    const uint_fast16_t y = static_cast<uint_fast16_t>(i / width);
    const float posX = startX + x * gap;
    const float posY = startY + y * gap;

    const std::array<float, 4> pos = {posX, posY, 0.0f, 0.0f};
    const bool isAlive = isAliveIndices[i];

    const std::array<float, 4>& color = isAlive ? blue : red;
    const std::array<int, 4>& state = isAlive ? alive : dead;

    cells[i] = {pos, color, size, state};
  }
  //_log.console(aliveCellIndices);
  return cells;
}

bool World::isIndexAlive(const std::vector<int>& aliveCells, int index) {
  return std::find(aliveCells.begin(), aliveCells.end(), index) !=
         aliveCells.end();
}

World::UniformBufferObject World::updateUniforms() {
  UniformBufferObject uniformObject{};
  uniformObject.gridDimensions = _control.grid.dimensions;
  uniformObject.gridHeight = _control.grid.height;
  uniformObject.passedHours = _control.timer.passedHours;
  uniformObject.cellSize = _control.cells.size;
  uniformObject.lightDirection = directionalLight.direction;
  uniformObject.model = setModel();
  uniformObject.view = setView();
  uniformObject.proj = setProjection(_mechanics.swapChain.extent);

  return uniformObject;
}

void World::updateCamera() {
  constexpr uint_fast8_t left = 0;
  constexpr uint_fast8_t right = 1;
  constexpr uint_fast8_t middle = 2;

  glm::vec2 buttonType[3]{};
  for (uint_fast8_t i = 0; i < 3; ++i) {
    buttonType[i] = _window.mouse.buttonDown[i].position -
                    _window.mouse.previousButtonDown[i].position;
    _window.mouse.previousButtonDown[i].position =
        _window.mouse.buttonDown[i].position;
  }

  float multiplier = 10.0f;
  glm::vec2 leftButtonDelta = buttonType[left];
  glm::vec2 rightButtonDelta = buttonType[right];
  glm::vec2 middleButtonDelta = buttonType[middle];

  constexpr float rotationSpeed = 0.3f * glm::pi<float>() / 180.0f;

  glm::vec3 cameraRight = glm::cross(camera.front, camera.up);

  // Calculate rotation around the cameraRight axis only if leftButtonDelta.y is
  // larger
  if (glm::abs(leftButtonDelta.y) > glm::abs(leftButtonDelta.x)) {
    glm::mat4 rotationMatrix = glm::rotate(
        glm::mat4(1.0f), rotationSpeed * -leftButtonDelta.y * multiplier,
        cameraRight);
    camera.front = glm::normalize(
        glm::vec3(rotationMatrix * glm::vec4(camera.front, 0.0f)));
    camera.up =
        glm::normalize(glm::vec3(rotationMatrix * glm::vec4(camera.up, 0.0f)));
  }
  // Calculate rotation around the camera.up axis only if leftButtonDelta.x is
  // larger
  else if (glm::abs(leftButtonDelta.x) > glm::abs(leftButtonDelta.y)) {
    glm::mat4 rotationMatrix =
        glm::rotate(glm::mat4(1.0f),
                    rotationSpeed * leftButtonDelta.x * multiplier, camera.up);
    camera.front = glm::normalize(
        glm::vec3(rotationMatrix * glm::vec4(camera.front, 0.0f)));
    camera.up =
        glm::normalize(glm::vec3(rotationMatrix * glm::vec4(camera.up, 0.0f)));
  }

  // Reset the roll component to maintain a level camera orientation
  camera.up = glm::cross(cameraRight, camera.front);

  constexpr float movementSpeed = 0.001f;
  static bool leftMouseButtonDown = false;
  static float forwardMovement = 0.0f;
  float leftButtonDeltaLength = glm::length(leftButtonDelta);

  if (leftButtonDeltaLength > 0.0f) {
    if (!leftMouseButtonDown) {
      leftMouseButtonDown = true;
      forwardMovement = 0.0f;
    }
    forwardMovement += movementSpeed * leftButtonDeltaLength;
  } else {
    leftMouseButtonDown = false;
    forwardMovement = 0.0f;
  }

  forwardMovement = glm::clamp(forwardMovement, 0.0f, 1.0f);
  camera.position += forwardMovement * camera.front;

  constexpr float panningSpeed = 0.5f;
  glm::vec3 cameraUp = glm::cross(cameraRight, camera.front);
  camera.position += panningSpeed * rightButtonDelta.x * -cameraRight;
  camera.position += panningSpeed * rightButtonDelta.y * -cameraUp;

  constexpr float zoomSpeed = 0.1f;
  camera.position += zoomSpeed * middleButtonDelta.x * camera.front;
}

glm::mat4 World::setModel() {
  glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f),
                                glm::vec3(0.0f, 0.0f, 1.0f));
  return model;
}

glm::mat4 World::setView() {
  updateCamera();
  glm::mat4 view;
  view =
      glm::lookAt(camera.position, camera.position + camera.front, camera.up);
  return view;
}

glm::mat4 World::setProjection(VkExtent2D& swapChainExtent) {
  float nearClipping = 0.0001f;
  float farClipping = 100.0f;
  glm::mat4 projection = glm::perspective(
      glm::radians(camera.fieldOfView),
      swapChainExtent.width / static_cast<float>(swapChainExtent.height),
      nearClipping, farClipping);

  projection[1][1] *= -1;  // flip y axis
  projection[0][0] *= -1;  // flip x axis

  return projection;
};
