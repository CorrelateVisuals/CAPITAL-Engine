#pragma once
// #include <vulkan/vulkan.h>
// #define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
//
// #include <cstdint>
// #include <vector>
#include <array>
#include <string>

struct DisplayConfig {
  const char* windowTitle = "CAPITAL engine";
  uint32_t width = 1920;
  uint32_t height = 1080;
};
inline DisplayConfig displayConfig{};
