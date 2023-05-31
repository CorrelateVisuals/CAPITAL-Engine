#pragma once

#include <glm/glm.hpp>

class Library {
 public:
  double lowFrequencyOsciallator();
  glm::vec2 smoothstep(const glm::vec2 xy);
};

inline Library lib;
