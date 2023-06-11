#pragma once
#include "Control.h"
#include "Debug.h"
#include "Mechanics.h"
#include "Memory.h"
#include "Pipelines.h"
#include "Window.h"
#include "World.h"

class CapitalEngine {
 public:
  CapitalEngine();
  ~CapitalEngine();

  void mainLoop();

 private:
  void compileShaders();
  void initVulkan();
  void drawFrame();
};

class Global {
 public:
  Global() = default;
  ~Global();

  class Objects {
   public:
    Objects() = default;
    ~Objects() = default;

    Logging logging;
    ValidationLayers validation;
    Control control;
    VulkanMechanics mechanics;
    Pipelines pipelines;
    Memory memory;
    Window mainWindow;
    World world;
  };
  inline static Objects obj;

 private:
  void cleanup();
};

inline static auto& _log = Global::obj.logging;
inline static auto& _validation = Global::obj.validation;
inline static auto& _window = Global::obj.mainWindow;
inline static auto& _mechanics = Global::obj.mechanics;
inline static auto& _pipelines = Global::obj.pipelines;
inline static auto& _memory = Global::obj.memory;
inline static auto& _control = Global::obj.control;
inline static auto& _world = Global::obj.world;
