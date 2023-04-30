#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <array>
#include <vector>

#include "Control.h"

const uint32_t PARTICLE_COUNT = 1024;

class World {
 public:
  World();
  ~World();

  struct Grid {
    const int width = 10;
    const int height = 10;
    const int numGridPoints = PARTICLE_COUNT;
  } grid;

  struct Cells {
    glm::vec2 position;
    glm::vec2 newVariableToShader;
    glm::vec4 color;

    static VkVertexInputBindingDescription getBindingDescription() {
      VkVertexInputBindingDescription bindingDescription{};
      bindingDescription.binding = 0;
      bindingDescription.stride = sizeof(Cells);
      bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3>
    getAttributeDescriptions() {
      std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

      attributeDescriptions[0].binding = 0;
      attributeDescriptions[0].location = 0;
      attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
      attributeDescriptions[0].offset = offsetof(Cells, position);

      attributeDescriptions[1].binding = 0;
      attributeDescriptions[1].location = 1;
      attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
      attributeDescriptions[1].offset = offsetof(Cells, color);

      attributeDescriptions[2].binding = 0;
      attributeDescriptions[2].location = 2;
      attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
      attributeDescriptions[2].offset = offsetof(Cells, newVariableToShader);

      return attributeDescriptions;
    }
  };
};
