#include <chrono>
#include <random>

#include "CAPITAL_Engine.h"
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

  if (elapsedSeconds >= (1.0 / simulationSpeed)) {
    passedSimulationHours++;
    lastTime = currentTime;
  }
}

float Control::getRandomFloat(float min, float max) {
  static std::mt19937 rng(std::random_device{}());
  static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
  return dist(rng) * (max - min) + min;
}

std::vector<int> Control::setCellsAliveRandomly(int size) {
  std::vector<int> CellIDs;

  while (CellIDs.size() < size) {
    int CellID = static_cast<int>(getRandomFloat(0, 1) *
                                  _control.grid.numberOfGridPoints);
    // check if the CellID is not already in CellIDs
    if (std::find(CellIDs.begin(), CellIDs.end(), CellID) == CellIDs.end()) {
      CellIDs.push_back(CellID);
    }
  }
  std::sort(CellIDs.begin(), CellIDs.end());
  return CellIDs;
}
