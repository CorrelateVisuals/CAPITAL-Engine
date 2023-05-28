#pragma once
#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <vector>

#include "Control.h"

class World {
 public:
  World();
  ~World();

  struct UniformBufferObject {
    int sqrtOfGrid;
    int passedHours;  // TODO: 'long long'
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    std::array<float, 4> lightDirection;
    float cellSize;
  };

  struct Cell {
    std::array<float, 4> position;
    std::array<float, 4> color;
    std::array<float, 4> size;
    std::array<int, 4> states;
  };

  struct Camera {
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.5f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, -1.0f, 0.0f);
  } camera;

 public:
  std::vector<World::Cell> initializeCells();
  bool isIndexAlive(const std::vector<int>& aliveCells, int index);

  UniformBufferObject updateUniforms();
  glm::mat4 setModel();
  glm::mat4 setView();
  glm::mat4 setProjection(VkExtent2D& swapChainExtent);

  static std::vector<VkVertexInputAttributeDescription>
  getAttributeDescriptions();
  static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
};
