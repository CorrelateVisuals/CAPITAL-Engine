#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <array>
#include <vector>

#include "Control.h"

class World {
 public:
  World();
  ~World();

  struct Grid {
    const int width = 32;
    const int height = 32;
    const int numGridPoints = width * height;
  } grid;

  struct Cell {
    // All cell parameters have to fit 16 byte memory blocks.
    // An array of 4 floats, 4 integers, or a single vec4 fits this size.
    // Multiple data types can fit in a block as well, for instance two vec2s.
    std::array<float, 4> position;  // xyz
    std::array<float, 4> color;     // rgba
    std::array<float, 4> size;      // 1 float
    std::array<float, 4> gridSize;  // 1 int

    static std::vector<VkVertexInputAttributeDescription>
    getAttributeDescriptions() {
      std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

      attributeDescriptions.push_back(
          {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Cell, position)});
      attributeDescriptions.push_back(
          {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Cell, color)});
      attributeDescriptions.push_back(
          {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Cell, size)});
      attributeDescriptions.push_back(
          {3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Cell, gridSize)});

      return attributeDescriptions;
    }

    static VkVertexInputBindingDescription getBindingDescription() {
      VkVertexInputBindingDescription bindingDescription{};
      bindingDescription.binding = 0;
      bindingDescription.stride = sizeof(Cell);
      bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return bindingDescription;
    }
  };
};
