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
    float speed = 5.0f;
    uint64_t passedHours = 0;
  } timer;

  struct Grid {
    uint_fast32_t totalAliveCells = 10000;
    std::array<uint_fast16_t, 2> dimensions = {200, 200};
    float height = 0.75f;
    int heightSteps = 15;
  } grid;

  struct DisplayConfiguration {
    const char* title = "CAPITAL Engine";
    uint16_t width = 3840;
    uint16_t height = 2041;
  } display;

  struct Compute {
    const uint8_t localSizeX = 8;
    const uint8_t localSizeY = 8;
  } compute;

 public:
  std::vector<uint_fast32_t> setCellsAliveRandomly(uint_fast32_t numberOfCells);
  void setPassedHours();

  void setPushConstants();
};
