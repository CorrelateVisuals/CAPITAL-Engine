#pragma once
#include "Control.h"
#include "Debug.h"
#include "Mechanics.h"
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
    ValidationLayers validationLayers;
    Control control;
    VulkanMechanics vulkanMechanics;
    Pipelines pipelines;
    MemoryCommands memoryCommands;
    Window mainWindow;
    World world;
  };
  inline static Objects obj;

 private:
  void cleanup();
};

inline static auto& _log = Global::obj.logging;
inline static auto& _validationLayers = Global::obj.validationLayers;
inline static auto& _window = Global::obj.mainWindow;
inline static auto& _mechanics = Global::obj.vulkanMechanics;
inline static auto& _pipelines = Global::obj.pipelines;
inline static auto& _memCommands = Global::obj.memoryCommands;
inline static auto& _control = Global::obj.control;
inline static auto& _world = Global::obj.world;
