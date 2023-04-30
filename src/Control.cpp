#include <chrono>

#include "Control.h"

double Control::timer() {
  static auto start_time = std::chrono::high_resolution_clock::now();
  auto current_time = std::chrono::high_resolution_clock::now();
  static double time = 0.0;
  static const double max_time = 1;
  static const int num_steps = 1024;
  static const double step_size = max_time / num_steps;

  std::chrono::duration<double> elapsed_time = current_time - start_time;
  double elapsed_seconds = elapsed_time.count();

  time = fmod(elapsed_seconds / max_time, 1024.0);
  int step = static_cast<int>(time / step_size);
  time = step * step_size;
  int intTime = static_cast<int>(time);
  return intTime;
}
