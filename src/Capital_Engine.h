#pragma once
#include "Debug.h"
#include "Pipelines.h"
#include "Vulkan_Mechanics.h"
#include "Window.h"
#include "World.h"

#define OBJECT Globals::obj
#define LOG OBJECT.logging.console
#define LOG_LAYERS OBJECT.validationLayers
#define WINDOW OBJECT.mainWindow
#define MECHANICS OBJECT.mechanics
#define RENDER_CONFIG OBJECT.renderConfig
#define MEM_COMMANDS OBJECT.memCommands
#define PIPELINES OBJECT.pipelines
#define WORLD OBJECT.world

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
    ~Objects(){};

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
