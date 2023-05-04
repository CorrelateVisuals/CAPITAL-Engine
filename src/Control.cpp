#include <chrono>

#include "CAPITAL_Engine.h"
#include "Control.h"
#include "World.h"

Control::Control() {
  _log.console("{ Ctr }", "constructing Control");
}

Control::~Control() {
  _log.console("{ Ctr }", "destructing Control");
}

double Control::timer() {
  static auto start_time = std::chrono::high_resolution_clock::now();
  auto current_time = std::chrono::high_resolution_clock::now();
  static double time = 0.0;
  static const double max_time = 1;
  static const int num_steps = _world.grid.numGridPoints;
  static const double step_size = max_time / num_steps;

  std::chrono::duration<double> elapsed_time = current_time - start_time;
  double elapsed_seconds = elapsed_time.count();

  time = fmod(elapsed_seconds / max_time, _world.grid.numGridPoints);
  int step = static_cast<int>(time / step_size);
  time = step * step_size;
  int intTime = static_cast<int>(time);
  return intTime;
}
