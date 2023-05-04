#pragma once

#include <array>
#include <string>

class Control {
 public:
  Control() = default;
  ~Control() = default;

  double timer();
};

struct DisplayConfig {
 public:
  const char* windowTitle = "CAPITAL Engine";
  uint32_t width = 1920;
  uint32_t height = 1080;
};
inline DisplayConfig displayConfig;
