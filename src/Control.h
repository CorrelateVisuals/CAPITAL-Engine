#pragma once

#include <array>
#include <string>

/// <TO DO>
/// Adding 'long long' to shader with the right extension on the Vulkan side
///
/// Adding vertices and indices to instance cubes instead of grid points
/// Adding model, view, projection matrices
///
/// Adding camera movement connected to the mouse
///
/// Moving one cell to another, than replacing the moving cell with another
/// object
///
/// ...
///
/// </TO DO>

class Control {
 public:
  Control();
  ~Control();

  long long passedSimulationHours = 0;
  float simulationSpeed = 100.0f;
  void simulateHours();
};

struct DisplayConfig {
 public:
  const char* windowTitle = "CAPITAL Engine";
  uint32_t width = 1920;
  uint32_t height = 1080;
};
inline DisplayConfig displayConfig;
