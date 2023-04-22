#include <iostream>

#include "Capital_Engine.h"
#include "Debug.h"
#include "Vulkan_Mechanics.h"
#include "Window.h"

CapitalEngine::CapitalEngine() {
  LOG("... constructing Capital Engine");
  initVulkan();
}

CapitalEngine::~CapitalEngine() {
  LOG("... destructing Capital Engine");
}

void CapitalEngine::mainLoop() {
  while (!glfwWindowShouldClose(mainWindow.window)) {
    glfwPollEvents();
    mainWindow.mouseClick(mainWindow.window, GLFW_MOUSE_BUTTON_LEFT);
  }
}

void CapitalEngine::initVulkan() {
  LOG("... initializing Capital Engine");

  // Vulkan Config
  vulkanMechanics.createInstance();
  debug.setupDebugMessenger(vulkanMechanics.instance);
  vulkanMechanics.createSurface();

  // GPU handling
  vulkanMechanics.pickPhysicalDevice();
  vulkanMechanics.createLogicalDevice();

  // Engine mechanics
  vulkanMechanics.createSwapChain();
  vulkanMechanics.createSyncObjects();
}
