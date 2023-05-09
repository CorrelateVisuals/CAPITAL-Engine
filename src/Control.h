#pragma once

#include <array>
#include <string>
#include <vector>

class Control {
 public:
  Control();
  ~Control();

  float simulationSpeed = 10.0f;

  struct Grid {
    const uint32_t numberOfAliveCells = 200;
    const uint32_t gridDimensions[2] = {100, 100};  // rows, columns
    const float gridPointDistance = 2;
    const uint32_t numGridPoints = gridDimensions[0] * gridDimensions[1];
  } grid;

 public:
  int passedSimulationHours = 0;  // TODO: 'long long'

  std::vector<int> setCellsAliveRandomly(int size);
  void simulateHours();
};

struct DisplayConfig {
 public:
  const char* windowTitle = "CAPITAL Engine";
  uint32_t width = 1080;
  uint32_t height = 1080;
};
inline DisplayConfig displayConfig;
