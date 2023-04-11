#pragma once
#include <string>

#define LOG logging.console

class Logging {
  // TODO:
  // - Log complete objects and varius types: int, float, glm::vec3..
  // - Write to a log file
  // - Add date and time before logging message

 public:
  void console(std::string log_in);
  void console(std::string log_in, float val1, float val2);
};

inline Logging logging;
