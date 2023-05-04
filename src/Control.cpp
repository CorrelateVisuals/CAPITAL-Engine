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
  static auto startTime = std::chrono::high_resolution_clock::now();
  auto currentTime = std::chrono::high_resolution_clock::now();
  static double time = 0.0;
  static const double maxTime = 1;
  static const int numSteps = _world.grid.numGridPoints;
  static const double stepSize = maxTime / numSteps;

  std::chrono::duration<double> elapsedTime = currentTime - startTime;
  double elapsedSeconds = elapsedTime.count();

  time = fmod(elapsedSeconds / maxTime, _world.grid.numGridPoints);
  int step = static_cast<int>(time / stepSize);
  time = step * stepSize;
  int intTime = static_cast<int>(time);
  return intTime;
}
