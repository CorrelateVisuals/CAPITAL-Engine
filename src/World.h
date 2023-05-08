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
    const int width = 100;
    const int height = width;
    const int numGridPoints = width * height;
    const float gridPointDistance = 2;
    const int numberOfAliveCells = 100;
  } grid;

  struct Cell {
    std::array<float, 4> position;   // xyz
    std::array<float, 4> color;      // rgba
    std::array<float, 4> size;       // 1 float
    std::array<float, 4> gridSize;   // 1 int
    std::array<float, 4> printGLSL;  // used to write print statements to buffer
    std::array<float, 4> alive;
  };

 public:
  std::vector<int> setCellsAliveRandomly(int size);
  static std::vector<VkVertexInputAttributeDescription>
  getAttributeDescriptions();
  static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
};
