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

constexpr auto& LOG = Globals::obj.logging;
constexpr auto& LOG_LAYERS = Globals::obj.validationLayers;
constexpr auto& WINDOW = Globals::obj.mainWindow;
constexpr auto& MECHANICS = Globals::obj.mechanics;
constexpr auto& RENDER_CONFIG = Globals::obj.renderConfig;
constexpr auto& MEM_COMMANDS = Globals::obj.memCommands;
constexpr auto& PIPELINES = Globals::obj.pipelines;
constexpr auto& WORLD = Globals::obj.world;
