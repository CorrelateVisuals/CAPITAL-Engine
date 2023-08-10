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
    float speed = 30.0f;
    uint64_t passedHours{0};
  } timer;

  struct Grid {
    uint_fast32_t totalAliveCells = 0;
    std::array<uint_fast16_t, 2> dimensions = {20, 20};
  } grid;

  struct DisplayConfiguration {
    const char* title{"CAPITAL Engine"};
    uint16_t width = 1920;
    uint16_t height = 1080;
  } display;

  struct Compute {
    const uint8_t localSizeX{32};
    const uint8_t localSizeY{32};
    const uint8_t localSizeZ{1};
  } compute;

 public:
  std::vector<uint_fast32_t> setCellsAliveRandomly(uint_fast32_t numberOfCells);
  void setPassedHours();

  void setPushConstants();
};
