#include <iostream>

#include "Capital_Engine.h"
#include "Debug.h"
#include "Pipelines.h"
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

  // Init Vulkan
  mechanics.createInstance();
  debug.setupDebugMessenger(mechanics.instance);
  mechanics.createSurface();

  // GPU handling
  mechanics.pickPhysicalDevice();
  mechanics.createLogicalDevice();

  // Engine mechanics
  mechanics.createSwapChain();

  // Renderer Config
  renderConfig.createDepthResources();
  renderConfig.createImageViews();
  renderConfig.createRenderPass();

  // Pipelines
  pipelines.createDescriptorSetLayout();
  // renderConfig.createComputeDescriptorSetLayout();
  pipelines.createGraphicsPipeline();
  pipelines.createComputePipeline();

  renderConfig.createFrameBuffer();

  // vulkanMechanics.createSyncObjects();
  // vulkanMechanics.createCommandPool();
  // vulkanMechanics.createCommandBuffers();
  // renderConfig.createPipelineCache();
  // renderConfig.setupFrameBuffer();
}
