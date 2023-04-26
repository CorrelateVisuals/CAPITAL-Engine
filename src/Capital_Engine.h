#pragma once
#include "Debug.h"
#include "Pipelines.h"
#include "Vulkan_Mechanics.h"
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
    VulkanMechanics mechanics;
    RenderConfiguration renderConfig;
    Pipelines pipelines;
    MemoryCommands memCommands;
    World world;
  };
  inline static Objects obj;
};

constexpr auto& _log = Globals::obj.logging;
constexpr auto& _validationLayers = Globals::obj.validationLayers;
constexpr auto& _window = Globals::obj.mainWindow;
constexpr auto& _mechanics = Globals::obj.mechanics;
constexpr auto& _renderConfig = Globals::obj.renderConfig;
constexpr auto& _memCommands = Globals::obj.memCommands;
constexpr auto& _pipelines = Globals::obj.pipelines;
constexpr auto& _world = Globals::obj.world;
