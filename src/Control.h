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
    float speed = 100.0f;
    int passedHours = 0;
  } timer;

  struct Grid {
    uint_fast32_t totalAliveCells = 5000;
    std::array<uint_fast16_t, 2> dimensions = {200, 150};
    float gap = 0.1f;
    float height = 0.1f;
  } grid;

  struct Cell {
    float size = {0.02f};
  } cells;

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
  void simulateHours();
};
