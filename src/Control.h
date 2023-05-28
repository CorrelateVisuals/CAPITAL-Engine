#pragma once

#include <array>
#include <string>
#include <vector>

constexpr int parameterOff = -1;

class Control {
 public:
  Control();
  ~Control();

  float simulationSpeed = 60.0f;

  struct Grid {
    const uint32_t numberOfAliveCells = 300;
    const std::array<uint32_t, 2> dimensions = {30, 30};
    const float distance = 3.5;
  } grid;

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

  // Globally accessible
  int passedSimulationHours = 0;  // TODO: 'long long'
};

struct DisplayConfig {
 public:
  const char* projectTitle = "CAPITAL Engine";
  uint32_t width = 1080;
  uint32_t height = 1080;
};
inline DisplayConfig displayConfig;
