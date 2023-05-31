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
    float speed = 10.0f;
    int passedHours = 0;
  } timer;

  struct Grid {
    uint_fast32_t totalAliveCells = 2500;
    std::array<uint_fast16_t, 2> dimensions = {100, 100};
    float gap = 0.01f;
    float height = 0.02f;
    std::array<float, 4> lightDirection = {0.075f, 0.0f, 0.5f, 0.2f};
  } grid;

  struct Cell {
    float size = {0.005f};
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
