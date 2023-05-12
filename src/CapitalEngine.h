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
    Window mainWindow;
    VulkanMechanics vulkanMechanics;
    Pipelines pipelines;
    MemoryCommands memoryCommands;
    Control control;
    World world;
  };
  inline static Objects obj;

 private:
  void cleanup();
};
// constexpr auto& _log = Global::obj.logging;
// constexpr auto& _validationLayers = Global::obj.validationLayers;
// constexpr auto& _window = Global::obj.mainWindow;
// constexpr auto& _mechanics = Global::obj.vulkanMechanics;
// constexpr auto& _pipelines = Global::obj.pipelines;
// constexpr auto& _memCommands = Global::obj.memoryCommands;
// constexpr auto& _control = Global::obj.control;
// constexpr auto& _world = Global::obj.world;

static constexpr auto& _log = Global::obj.logging;
static constexpr auto& _validationLayers = Global::obj.validationLayers;
static constexpr auto& _window = Global::obj.mainWindow;
static constexpr auto& _mechanics = Global::obj.vulkanMechanics;
static constexpr auto& _pipelines = Global::obj.pipelines;
static constexpr auto& _memCommands = Global::obj.memoryCommands;
static constexpr auto& _control = Global::obj.control;
static constexpr auto& _world = Global::obj.world;
