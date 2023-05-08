#pragma once

#include <array>
#include <string>

class Control {
 public:
  Control();
  ~Control();

  int passedSimulationHours = 0;  // TODO: 'long long'
  float simulationSpeed = 10.0f;
  void simulateHours();
};

struct DisplayConfig {
 public:
  const char* windowTitle = "CAPITAL Engine";
  uint32_t width = 1920;
  uint32_t height = 1080;
};
inline DisplayConfig displayConfig;
