#pragma once

#include <array>
#include <string>

class Control {
 public:
  double timer();
};

struct DisplayConfig {
 public:
  const char* windowTitle = "CAPITAL engine";
  uint32_t width = 1920;
  uint32_t height = 1080;
};
inline DisplayConfig displayConfig;
