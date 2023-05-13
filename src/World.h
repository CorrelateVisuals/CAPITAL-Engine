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
    int passedHours;  // TODO: 'long long'
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    std::array<float, 4> lightDirection;
  };

  struct Cell {
    std::array<float, 4> position;   // xyz pos
    std::array<float, 4> color;      // rgba
    std::array<float, 4> size;       // 1 float
    std::array<float, 4> endOfTurn;  // bool
  };

 public:
  static std::vector<VkVertexInputAttributeDescription>
  getAttributeDescriptions();
  static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();

  glm::mat4 setModel();
  glm::mat4 setView();
  glm::mat4 setProjection(VkExtent2D& swapChainExtent);
};
