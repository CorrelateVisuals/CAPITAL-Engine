#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "CapitalEngine.h"
#include "Library.h"
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
  std::vector<VkVertexInputBindingDescription> bindingDescriptions{
      {0, sizeof(Cell), VK_VERTEX_INPUT_RATE_INSTANCE}};
  return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription>
World::getAttributeDescriptions() {
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions{
      {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Cell, position)},
      {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Cell, color)},
      {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Cell, size)},
      {3, 0, VK_FORMAT_R32G32B32_SINT, offsetof(Cell, states)}};
  return attributeDescriptions;
}

std::vector<World::Cell> World::initializeCells() {
  const uint_fast16_t width = _control.grid.dimensions[0];
  const uint_fast16_t height = _control.grid.dimensions[1];
  const uint_fast32_t numGridPoints = width * height;
  const uint_fast32_t numAliveCells = _control.grid.totalAliveCells;
  const float gap = _control.grid.gap;
  std::array<float, 4> size = {tile.size};

  if (numAliveCells > numGridPoints) {
    throw std::runtime_error(
        "\n!ERROR! Number of alive cells exceeds number of grid "
        "points");
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
  return cells;
}

bool World::isIndexAlive(const std::vector<int>& aliveCells, int index) {
  return std::find(aliveCells.begin(), aliveCells.end(), index) !=
         aliveCells.end();
}

World::UniformBufferObject World::updateUniforms() {
  UniformBufferObject uniformObject{
      .light = light.position,
      .gridDimensions = {static_cast<uint32_t>(_control.grid.dimensions[0]),
                         static_cast<uint32_t>(_control.grid.dimensions[1])},
      .gridHeight = _control.grid.height,
      .cellSize = tile.size,
      .model = setModel(),
      .view = setView(),
      .proj = setProjection(_mechanics.swapChain.extent)};

  return uniformObject;
}

void World::updateCamera() {
  glm::vec2 buttonType[3]{};
  constexpr uint_fast8_t left = 0;
  constexpr uint_fast8_t right = 1;
  constexpr uint_fast8_t middle = 2;
  bool mousePositionChanged = false;
  static bool run = false;

  for (uint_fast8_t i = 0; i < 3; ++i) {
    buttonType[i] = _window.mouse.buttonDown[i].position -
                    _window.mouse.previousButtonDown[i].position;

    if (_window.mouse.buttonDown[i].position !=
        _window.mouse.previousButtonDown[i].position) {
      mousePositionChanged = true;
      _window.mouse.previousButtonDown[i].position =
          _window.mouse.buttonDown[i].position;
    }
  }
  if (mousePositionChanged) {
    run = mousePositionChanged;
  }

  if (run) {
    glm::vec2 leftButtonDelta = buttonType[left];
    glm::vec2 rightButtonDelta = buttonType[right];
    glm::vec2 middleButtonDelta = buttonType[middle];
    glm::vec3 cameraRight = glm::cross(camera.front, camera.up);
    glm::vec2 absDelta = glm::abs(leftButtonDelta);
    constexpr float rotationSpeed = 0.2f * glm::pi<float>() / 180.0f;
    glm::vec2 rotationDelta = rotationSpeed * leftButtonDelta;
    glm::mat4 rotationMatrix(1.0f);

    if (absDelta.y > absDelta.x) {
      rotationMatrix =
          glm::rotate(rotationMatrix, -rotationDelta.y, cameraRight);
    } else if (absDelta.x > absDelta.y) {
      rotationMatrix = glm::rotate(rotationMatrix, rotationDelta.x, camera.up);
    }
    camera.front = glm::normalize(
        glm::vec3(rotationMatrix * glm::vec4(camera.front, 0.0f)));
    camera.up =
        glm::normalize(glm::vec3(rotationMatrix * glm::vec4(camera.up, 0.0f)));

    float movementSpeed = getForwardMovement(leftButtonDelta);
    camera.position += movementSpeed * camera.front;

    constexpr float panningSpeed = 0.02f;
    glm::vec3 cameraUp = glm::cross(cameraRight, camera.front);
    camera.position -= panningSpeed * rightButtonDelta.x * cameraRight;
    camera.position -= panningSpeed * rightButtonDelta.y * cameraUp;

    constexpr float zoomSpeed = 0.02f;
    camera.position += zoomSpeed * middleButtonDelta.x * camera.front;
    camera.position.z = std::max(camera.position.z, 0.0f);
  }
  run = mousePositionChanged;
}

float World::getForwardMovement(const glm::vec2& leftButtonDelta) {
  static bool leftMouseButtonDown = false;
  static float forwardMovement = 0.0f;
  float leftButtonDeltaLength = glm::length(leftButtonDelta);

  if (leftButtonDeltaLength > 0.0f) {
    if (!leftMouseButtonDown) {
      leftMouseButtonDown = true;
      forwardMovement = 0.0f;
    }
    constexpr float maxSpeed = 0.002f;
    constexpr float acceleration = 0.00001f;

    // Calculate the speed based on the distance from the center
    float normalizedDeltaLength = glm::clamp(leftButtonDeltaLength, 0.0f, 1.0f);
    float targetSpeed =
        glm::smoothstep(0.0f, maxSpeed, 1.0f - normalizedDeltaLength);
    forwardMovement += acceleration * (targetSpeed - forwardMovement);
    forwardMovement = glm::clamp(forwardMovement, 0.0f, maxSpeed);
  } else {
    leftMouseButtonDown = false;
    forwardMovement = 0.0f;
  }
  return forwardMovement;
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
  float farClipping = 1000.0f;
  glm::mat4 projection = glm::perspective(
      glm::radians(camera.fieldOfView),
      swapChainExtent.width / static_cast<float>(swapChainExtent.height),
      nearClipping, farClipping);

  projection[1][1] *= -1;  // flip y axis
  projection[0][0] *= -1;  // flip x axis

  return projection;
};
