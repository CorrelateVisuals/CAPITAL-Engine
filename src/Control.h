#pragma once

#include <array>
#include <string>
#include <vector>

constexpr int parameterOff = -1;

class Control {
 public:
  Control() = default;
  ~Control() = default;

  float simulationSpeed = 60.0f;

  struct Grid {
    const uint32_t totalAliveCells = 100;
    const std::array<uint32_t, 2> dimensions = {30, 10};
    const float distance = 0.1f;
  } grid;

  struct DisplayConfiguration {
    const char* title = "CAPITAL Engine";
    uint32_t width = 1080;
    uint32_t height = 1080;
  } display;

 public:
  struct Compute {
    uint32_t localSizeX = 8;
    uint32_t localSizeY = 8;
  } compute;

  // Initialization
  float getRandomFloat(float min, float max);
  std::vector<int> setCellsAliveRandomly(size_t size);

  // Animation
  double lowFrequencyOsciallator();
  void simulateHours();

  // Globally accessible
  int passedSimulationHours = 0;  // TODO: 'long long'
};
