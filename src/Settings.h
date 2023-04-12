#pragma once

#include <array>
#include <string>

struct DisplayConfig {
  const char* windowTitle = "CAPITAL engine";
  uint32_t width = 1920;
  uint32_t height = 1080;
};
inline DisplayConfig displayConfig{};

struct Settings {
 public:
  /** @brief Activates validation layers (and message output) when set to true
   */
  bool validation = false;
};
inline Settings settings;
