#pragma once

#include <glm/glm.hpp>

class Library {
 public:
  std::vector<float> generateRandomValues(int amount, float min, float max);

  double lowFrequencyOsciallator();
  glm::vec2 smoothstep(const glm::vec2 xy);
};

inline Library lib;
