#pragma once

#include <array>
#include <string>
#include <vector>

class Control {
 public:
  Control();
  ~Control();

  int passedSimulationHours = 0;  // TODO: 'long long'
  float simulationSpeed = 10.0f;

  struct Grid {
    const int width = 20;
    const int height = width;
    const int numGridPoints = width * height;
    const float gridPointDistance = 2;
    const int numberOfAliveCells = 300;
  } grid;

 public:
  std::vector<int> setCellsAliveRandomly(int size);
  void simulateHours();
};

struct DisplayConfig {
 public:
  const char* windowTitle = "CAPITAL Engine";
  uint32_t width = 1920;
  uint32_t height = 1080;
};
inline DisplayConfig displayConfig;
