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
    float speed = 1.0f;
    uint64_t passedHours = 0;
  } timer;

  struct Grid {
    uint_fast32_t totalAliveCells = 500;
    std::array<uint_fast16_t, 2> dimensions = {100, 100};
    float height = 0.3f;
    const float gap = 0.6f;
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
  std::vector<uint_fast32_t> setCellsAliveRandomly(uint_fast32_t numberOfCells);
  void setPassedHours();

  void setPushConstants();
};
