#include <chrono>
#include <numbers>
#include <random>
#include <unordered_set>

#include "CapitalEngine.h"
#include "Control.h"
#include "World.h"

Control::Control() {
  _log.console("{ CTR }", "constructing Control");
}

Control::~Control() {
  _log.console("{ CTR }", "destructing Control");
}

void Control::simulateHours() {
  static auto lastTime = std::chrono::system_clock::now();
  auto currentTime = std::chrono::system_clock::now();
  auto elapsedSeconds = std::chrono::duration_cast<std::chrono::microseconds>(
                            currentTime - lastTime)
                            .count() /
                        1000000.0;

  if (elapsedSeconds >= (1.0 / timer.speed)) {
    passedSimulationHours++;
    lastTime = currentTime;
  }
}

double Control::lowFrequencyOsciallator() {
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

std::vector<int> Control::setCellsAliveRandomly(size_t size) {
  std::vector<int> CellIDs;
  CellIDs.reserve(size);

  std::random_device random;
  std::mt19937 generate(random());
  std::uniform_int_distribution<int> distribution(
      0, _control.grid.dimensions[0] * _control.grid.dimensions[1] - 1);

  while (CellIDs.size() < size) {
    int CellID = distribution(generate);
    if (std::find(CellIDs.begin(), CellIDs.end(), CellID) == CellIDs.end()) {
      CellIDs.push_back(CellID);
    }
  }
  std::sort(CellIDs.begin(), CellIDs.end());
  return CellIDs;
}
