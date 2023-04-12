#include <iostream>
#include <string>

#include "Debug.h"

void Logging::console(std::string log_in) {
  std::string log_out = log_in;
  std::cerr << log_in << std::endl;
}

void Logging::console(std::string log_in, float val1, float val2) {
  std::string values = std::to_string(val1) + " " + std::to_string(val2);
  std::string log_out = log_in + " " + values;
  std::cerr << log_out << std::endl;
}
