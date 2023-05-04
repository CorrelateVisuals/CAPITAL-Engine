#pragma once

#include <array>
#include <string>

class Control {
 public:
  Control();
  ~Control();

  long long simulationHours;

  void updateSimulation(long long& simulationHours,
                        double speed,
                        long long duration = -1);

  double simulationTimer();
};

struct DisplayConfig {
 public:
  const char* windowTitle = "CAPITAL Engine";
  uint32_t width = 1920;
  uint32_t height = 1080;
};
inline DisplayConfig displayConfig;
