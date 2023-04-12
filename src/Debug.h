#pragma once

#include <string>

#define LOG logging.console

class Logging {
  // TODO:
  // - Log complete objects and varius types: int, float, glm::vec3..
  // - Write to a log file
  // - Add date and time before logging message

 public:
  template <class... Ts>
  void console(Ts&&... inputs) {
    int i = 0;
    (
        [&] {
          ++i;
          std::cerr << " " << inputs;
        }(),
        ...);
    std::cerr << std::endl;
  }
};

inline Logging logging;
