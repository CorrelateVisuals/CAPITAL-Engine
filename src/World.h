#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <array>
#include <vector>

#include "Settings.h"

constexpr int CELL_COUNT = 4;

class World {
 public:
  World();
  ~World();

  struct Cell {
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec4 color;

    static VkVertexInputBindingDescription getBindingDescription() {
      VkVertexInputBindingDescription bindingDescription{};
      bindingDescription.binding = 0;
      bindingDescription.stride = sizeof(Cell);
      bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2>
    getAttributeDescriptions() {
      std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

      attributeDescriptions[0].binding = 0;
      attributeDescriptions[0].location = 0;
      attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
      attributeDescriptions[0].offset = offsetof(Cell, position);

      attributeDescriptions[1].binding = 0;
      attributeDescriptions[1].location = 1;
      attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
      attributeDescriptions[1].offset = offsetof(Cell, color);

      return attributeDescriptions;
    }
  };
};
