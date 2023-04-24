#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <array>
#include <vector>

#include "Settings.h"

class World {
 public:
  World();
  ~World();

  struct Cell {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 velocity;

    glm::vec3 normals;
    glm::vec3 uv;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::vector<VkVertexInputAttributeDescription>
    getAttributeDescriptions();
  };
};
inline World world;
