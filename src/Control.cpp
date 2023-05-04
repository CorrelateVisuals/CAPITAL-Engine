#include <chrono>
#include <thread>

#include "CAPITAL_Engine.h"
#include "Control.h"
#include "World.h"

Control::Control() {
  _log.console("{ Ctr }", "constructing Control");
}

Control::~Control() {
  _log.console("{ Ctr }", "destructing Control");
}

#include <chrono>
#include <thread>

// Update simulationHours variable with an adaptable speed and an optional
// duration
void updateSimulation(long long& simulationHours,
                      double speed,
                      long long duration = -1) {
  auto startTime = std::chrono::high_resolution_clock::now();
  long long elapsedHours = 0;

  while (duration < 0 || elapsedHours < duration) {
    // Calculate elapsed time since last update
    auto currentTime = std::chrono::high_resolution_clock::now();
    double elapsedTime = std::chrono::duration_cast<std::chrono::microseconds>(
                             currentTime - startTime)
                             .count() /
                         1000000.0;
    startTime = currentTime;

    // Update simulation time based on speed and elapsed time
    simulationHours += static_cast<long long>(speed * elapsedTime);

    // Calculate elapsed hours
    elapsedHours = simulationHours;

    // Sleep for a short time to avoid busy waiting
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

double Control::simulationTimer() {
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
