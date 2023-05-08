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
    const float gridPointDistance = 2;
  } grid;

  struct Cell {
    // All cell parameters have to fit 16 byte memory blocks.
    // An array of 4 floats, 4 integers, or a single vec4 fits this size.
    // Multiple data types can fit in a block as well, for instance two vec2s.
    std::array<float, 4> position;   // xyz
    std::array<float, 4> color;      // rgba
    std::array<float, 4> size;       // 1 float
    std::array<float, 4> gridSize;   // 1 int
    std::array<float, 4> printGLSL;  // used to write print statements to buffer
    std::array<float, 4> alive;
    // std::array<float, 4> alive;      // 1 int

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
      attributeDescriptions.push_back(
          {4, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Cell, printGLSL)});

      return attributeDescriptions;
    }
    static VkVertexInputBindingDescription getBindingDescription();
  };

  std::vector<int> setCellsAliveRandomly(int size);

  struct Cube {
    std::array<float, 4> position;  // xyz
  };

  inline static const std::vector<Cube> cubeVertices = {
      {-1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f}, {-1.0f, 1.0f, -1.0f},
      {1.0f, 1.0f, -1.0f},   {-1.0f, -1.0f, 1.0f}, {1.0f, -1.0f, 1.0f},
      {-1.0f, 1.0f, 1.0f},   {1.0f, 1.0f, 1.0f}};

  inline static const std::vector<uint16_t> cubeIndices = {
      0, 2, 3, 0, 3, 1, 4, 5, 7, 4, 7, 6, 1, 3, 7, 1, 7, 5,
      0, 4, 6, 0, 6, 2, 2, 6, 7, 2, 7, 3, 0, 1, 5, 0, 5, 4,
  };
};
