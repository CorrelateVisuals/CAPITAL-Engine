#pragma once
#include "Debug.h"
#include "Window.h"

class Globals {
 public:
  class GlobalObjects {
   public:
    Logging logging;
    ValidationLayers debug;
    Window mainWindow;
  };
  inline static GlobalObjects obj;
};  // namespace Globals

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
