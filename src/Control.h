#pragma once

#include <array>
#include <string>
#include <vector>

class Control {
 public:
  Control();
  ~Control();

  float simulationSpeed = 10.0f;

  struct cellGeo {
    float size = 0.04f;
  } cellGeo;

  struct Grid {
    const uint32_t numberOfAliveCells = 400;
    const uint32_t gridDimensions[2] = {20, 20};  // rows, columns
    const float gridPointDistance = 3.5;
    const uint32_t numberOfGridPoints = gridDimensions[0] * gridDimensions[1];
  } grid;

 public:
  int passedSimulationHours = 0;  // TODO: 'long long'

  std::vector<int> setCellsAliveRandomly(int size);
  void simulateHours();
  float getRandomFloat(float min, float max);
};

struct DisplayConfig {
 public:
  const char* windowTitle = "CAPITAL Engine";
  uint32_t width = 1080;
  uint32_t height = 1080;
};
inline DisplayConfig displayConfig;
