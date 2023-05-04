#include <time.h>
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

void Control::simulateHours() {
  static auto lastTime = std::chrono::system_clock::now();
  auto currentTime = std::chrono::system_clock::now();
  auto elapsedSeconds = std::chrono::duration_cast<std::chrono::microseconds>(
                            currentTime - lastTime)
                            .count() /
                        1000000.0;

  if (elapsedSeconds >= (1.0 / simulationSpeed)) {
    passedSimulationHours++;
    lastTime = currentTime;
  }
}
