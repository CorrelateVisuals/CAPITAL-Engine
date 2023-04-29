#pragma once
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
  void initVulkan();
  void drawFrame();
};

class Global {
 public:
  Global() = default;
  ~Global();

  void cleanup();

  class Objects {
   public:
    Objects() = default;
    ~Objects() = default;

    Logging logging;
    ValidationLayers validationLayers;
    Window mainWindow;
    VulkanMechanics vulkanMechanics;
    Pipelines pipelines;
    MemoryCommands memoryCommands;
    World world;
  };
  inline static Objects obj;
};

constexpr auto& _log = Global::obj.logging;
constexpr auto& _validationLayers = Global::obj.validationLayers;
constexpr auto& _window = Global::obj.mainWindow;
constexpr auto& _mechanics = Global::obj.vulkanMechanics;
constexpr auto& _memCommands = Global::obj.memoryCommands;
constexpr auto& _pipelines = Global::obj.pipelines;
constexpr auto& _world = Global::obj.world;
