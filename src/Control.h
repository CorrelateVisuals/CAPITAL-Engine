#pragma once

#include <array>
#include <string>
#include <vector>

constexpr int parameterOff = -1;

class Control {
 public:
  Control();
  ~Control();

  struct Grid {
    float cellSize = 0.05f;
    const uint32_t numberOfAliveCells = 2000;
    const uint32_t gridDimensions[2] = {100, 100};  // only square for now
    const float gridPointDistance = 3.5;
    const uint32_t numberOfGridPoints = gridDimensions[0] * gridDimensions[1];
  } grid;

  float simulationSpeed = 1000.0f;

  struct Compute {
    uint32_t localSizeX = 8;
    uint32_t localSizeY = 8;
  } compute;

 public:
  // Initialization
  float getRandomFloat(float min, float max);
  std::vector<int> setCellsAliveRandomly(size_t size);

  // Animation
  double lowFrequencyOsciallator();
  void simulateHours();

 public:
  // Globally accessible variables
  int passedSimulationHours = 0;  // TODO: 'long long'
};

struct DisplayConfig {
 public:
  const char* projectTitle = "CAPITAL Engine";
  uint32_t width = 1080;
  uint32_t height = 1080;
};
inline DisplayConfig displayConfig;
