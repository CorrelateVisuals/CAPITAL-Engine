#pragma once

class CapitalEngine {
 public:
  CapitalEngine();
  ~CapitalEngine();

  void mainLoop();
  void cleanup();

 private:
  void initVulkan();
  void drawFrame();
};
