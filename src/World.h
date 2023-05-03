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
    const uint32_t width = 32;
    const uint32_t height = 32;
    const uint32_t numGridPoints = width * height;
  } grid;

  struct Cells {
    // All cell parameters have to fit 16 byte memory blocks.
    // An array of 4 floats, 4 integers, or a single vec4 fits this size.
    // Multiple data types can fit in a block as well, for instance two vec2s.
    std::array<float, 4> position;       // xyz
    std::array<float, 4> color;          // rgba
    std::array<float, 4> size;           // 1 float
    std::array<float, 4> substractGrid;  // 1 int

    static VkVertexInputBindingDescription getBindingDescription() {
      VkVertexInputBindingDescription bindingDescription{};
      bindingDescription.binding = 0;
      bindingDescription.stride = sizeof(Cells);
      bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 4>
    getAttributeDescriptions() {
      std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

      attributeDescriptions[0].binding = 0;
      attributeDescriptions[0].location = 0;
      attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
      attributeDescriptions[0].offset = offsetof(Cells, position);

      attributeDescriptions[1].binding = 0;
      attributeDescriptions[1].location = 1;
      attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
      attributeDescriptions[1].offset = offsetof(Cells, color);

      attributeDescriptions[2].binding = 0;
      attributeDescriptions[2].location = 2;
      attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
      attributeDescriptions[2].offset = offsetof(Cells, size);

      attributeDescriptions[3].binding = 0;
      attributeDescriptions[3].location = 3;
      attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
      attributeDescriptions[3].offset = offsetof(Cells, substractGrid);

      return attributeDescriptions;
    }
  };
};
