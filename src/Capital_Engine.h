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

class Globals {
 public:
  Globals() = default;
  ~Globals();

  void cleanup();

  class Objects {
   public:
    Objects() = default;
    ~Objects() = default;

    Logging logging;
    ValidationLayers validationLayers;
    Window mainWindow;
    VulkanMechanics vulkanMechanics;
    RenderConfiguration renderConfig;
    Pipelines pipelines;
    MemoryCommands memoryCommands;
    World world;
  };
  inline static Objects obj;
};

constexpr auto& _log = Globals::obj.logging;
constexpr auto& _validationLayers = Globals::obj.validationLayers;
constexpr auto& _window = Globals::obj.mainWindow;
constexpr auto& _mechanics = Globals::obj.vulkanMechanics;
constexpr auto& _renderConfig = Globals::obj.renderConfig;
constexpr auto& _memCommands = Globals::obj.memoryCommands;
constexpr auto& _pipelines = Globals::obj.pipelines;
constexpr auto& _world = Globals::obj.world;
