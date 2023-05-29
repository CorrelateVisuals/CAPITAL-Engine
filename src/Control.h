#pragma once

#include <array>
#include <string>
#include <vector>

constexpr int parameterOff = -1;

class Control {
 public:
  Control();
  ~Control();

  struct Timer {
    float speed = 40.0f;
  } timer;

  struct Grid {
    uint32_t totalAliveCells = 1000;
    std::array<uint32_t, 2> dimensions = {60, 30};
    float gap = 0.12f;
  } grid;

  struct DisplayConfiguration {
    const char* title = "CAPITAL Engine";
    uint32_t width = 1080;
    uint32_t height = 1080;
  } display;

  struct Compute {
    const uint32_t localSizeX = 8;
    const uint32_t localSizeY = 8;
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
