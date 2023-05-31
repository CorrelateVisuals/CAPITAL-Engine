#include <glm/glm.hpp>

#include <chrono>
#include <numbers>

#include "Library.h"

double Library::lowFrequencyOsciallator() {
  using namespace std::chrono;
  static const auto start_time = high_resolution_clock::now();
  const auto time_elapsed =
      duration_cast<milliseconds>(high_resolution_clock::now() - start_time)
          .count();
  const double period = 1000.0;             // time period in milliseconds
  const double frequency = 100.0 / period;  // frequency in Hz
  const double angle = time_elapsed * frequency * 2 * std::numbers::pi /
                       1000.0;  // angle in radians
  return 0.5 *
         (1 + std::sin(angle));  // lowFrequencyOsciallators between 0 and 1
}

glm::vec2 Library::smoothstep(const glm::vec2 xy) {
  float startInput = 0.0f;
  float endInput = 1.0f;
  float minIncrease = -0.1f;
  float maxIncrease = 0.1f;
  float tX =
      glm::clamp((xy.x - startInput) / (endInput - startInput), -1.0f, 1.0f);
  float tY =
      glm::clamp((xy.y - startInput) / (endInput - startInput), -1.0f, 1.0f);

  float smoothX = tX * tX * (3.0f - 2.0f * tX);
  float smoothY = tY * tY * (3.0f - 2.0f * tY);

  glm::vec2 increase =
      glm::vec2(smoothX, smoothY) * (maxIncrease - minIncrease) + minIncrease;

  return increase;
}
