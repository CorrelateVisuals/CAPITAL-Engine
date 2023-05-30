#pragma once

#include <array>
#include <string>
#include <vector>

constexpr int off = -1;

class Control {
 public:
  Control();
  ~Control();

  struct Timer {
    float speed = 40.0f;
    int passedHours = 0;
  } timer;

  struct Grid {
    uint32_t totalAliveCells = 700;
    std::array<uint32_t, 2> dimensions = {60, 30};
    float gap = 0.12f;
  } grid;

  struct DisplayConfiguration {
    const char* title = "CAPITAL Engine";
    uint16_t width = 1920;
    uint16_t height = 1080;
  } display;

  struct Compute {
    const uint8_t localSizeX = 8;
    const uint8_t localSizeY = 8;
  } compute;

 public:
  std::vector<int> setCellsAliveRandomly(uint32_t numberOfCells);

  double lowFrequencyOsciallator();
  void simulateHours();
};
