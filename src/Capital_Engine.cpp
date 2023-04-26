#include <iostream>

#include "Capital_Engine.h"
#include "Debug.h"
#include "Pipelines.h"
#include "Vulkan_Mechanics.h"
#include "Window.h"

CapitalEngine::CapitalEngine() {
  LOG("\n                    [", "Starting CAPITAL engine", "]\n");

  initVulkan();
}

CapitalEngine::~CapitalEngine() {
  LOG("\n                    [", "Terminating CAPITAL engine", "]\n");
}

void CapitalEngine::mainLoop() {
  LOG("{ main }", "running ...");
  while (!glfwWindowShouldClose(WINDOW.window)) {
    glfwPollEvents();
    drawFrame();
    WINDOW.mouseClick(WINDOW.window, GLFW_MOUSE_BUTTON_LEFT);
  }
  LOG("{ main }", "terminated");
}

void CapitalEngine::initVulkan() {
  LOG("{ ** }", "initializing Capital Engine");

  // Init Vulkan
  MECHANICS.createInstance();
  LOG_LAYERS.setupDebugMessenger(MECHANICS.instance);
  MECHANICS.createSurface();

  // GPU handling
  MECHANICS.pickPhysicalDevice();
  MECHANICS.createLogicalDevice();

  // Engine mechanics
  MECHANICS.createSwapChain();

  // Renderer Config
  RENDER_CONFIG.createDepthResources();
  RENDER_CONFIG.createImageViews();
  RENDER_CONFIG.createRenderPass();

  // Pipelines
  PIPELINES.createDescriptorSetLayout();
  PIPELINES.createGraphicsPipeline();
  PIPELINES.createComputePipeline();

  RENDER_CONFIG.createFrameBuffers();

  MEM_COMMANDS.createCommandPool();
  MEM_COMMANDS.createShaderStorageBuffers();
  MEM_COMMANDS.createUniformBuffers();

  MEM_COMMANDS.createDescriptorPool();
  MEM_COMMANDS.createComputeDescriptorSets();

  MEM_COMMANDS.createCommandBuffers();
  MEM_COMMANDS.createComputeCommandBuffers();

  MECHANICS.createSyncObjects();
}

void CapitalEngine::drawFrame() {
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  // Compute submission
  vkWaitForFences(MECHANICS.mainDevice.logical, 1,
                  &MECHANICS.computeInFlightFences[MECHANICS.currentFrame],
                  VK_TRUE, UINT64_MAX);

  MEM_COMMANDS.updateUniformBuffer(MECHANICS.currentFrame);

  vkResetFences(MECHANICS.mainDevice.logical, 1,
                &MECHANICS.computeInFlightFences[MECHANICS.currentFrame]);

  vkResetCommandBuffer(
      MEM_COMMANDS.computeCommandBuffers[MECHANICS.currentFrame],
      /*VkCommandBufferResetFlagBits*/ 0);
  MEM_COMMANDS.recordComputeCommandBuffer(
      MEM_COMMANDS.computeCommandBuffers[MECHANICS.currentFrame]);

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers =
      &MEM_COMMANDS.computeCommandBuffers[MECHANICS.currentFrame];
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores =
      &MECHANICS.computeFinishedSemaphores[MECHANICS.currentFrame];

  if (vkQueueSubmit(MECHANICS.queues.compute, 1, &submitInfo,
                    MECHANICS.computeInFlightFences[MECHANICS.currentFrame]) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to submit compute command buffer!");
  };

  // Graphics submission
  vkWaitForFences(MECHANICS.mainDevice.logical, 1,
                  &MECHANICS.inFlightFences[MECHANICS.currentFrame], VK_TRUE,
                  UINT64_MAX);

  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(
      MECHANICS.mainDevice.logical, MECHANICS.swapChain, UINT64_MAX,
      MECHANICS.imageAvailableSemaphores[MECHANICS.currentFrame],
      VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    MECHANICS.recreateSwapChain();
    return;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  vkResetFences(MECHANICS.mainDevice.logical, 1,
                &MECHANICS.inFlightFences[MECHANICS.currentFrame]);

  vkResetCommandBuffer(MEM_COMMANDS.commandBuffers[MECHANICS.currentFrame],
                       /*VkCommandBufferResetFlagBits*/ 0);
  MEM_COMMANDS.recordCommandBuffer(
      MEM_COMMANDS.commandBuffers[MECHANICS.currentFrame], imageIndex);

  VkSemaphore waitSemaphores[] = {
      MECHANICS.computeFinishedSemaphores[MECHANICS.currentFrame],
      MECHANICS.imageAvailableSemaphores[MECHANICS.currentFrame]};
  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  submitInfo.waitSemaphoreCount = 2;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers =
      &MEM_COMMANDS.commandBuffers[MECHANICS.currentFrame];
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores =
      &MECHANICS.renderFinishedSemaphores[MECHANICS.currentFrame];

  if (vkQueueSubmit(MECHANICS.queues.graphics, 1, &submitInfo,
                    MECHANICS.inFlightFences[MECHANICS.currentFrame]) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores =
      &MECHANICS.renderFinishedSemaphores[MECHANICS.currentFrame];

  VkSwapchainKHR swapChains[] = {MECHANICS.swapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;

  presentInfo.pImageIndices = &imageIndex;

  result = vkQueuePresentKHR(MECHANICS.queues.present, &presentInfo);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      WINDOW.framebufferResized) {
    WINDOW.framebufferResized = false;
    MECHANICS.recreateSwapChain();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  MECHANICS.currentFrame = (MECHANICS.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Globals::cleanup() {
  LOG("{ cleanup }");

  MECHANICS.cleanupSwapChain();

  vkDestroyPipeline(MECHANICS.mainDevice.logical, PIPELINES.graphics.pipeline,
                    nullptr);
  vkDestroyPipelineLayout(MECHANICS.mainDevice.logical,
                          PIPELINES.graphics.pipelineLayout, nullptr);

  vkDestroyPipeline(MECHANICS.mainDevice.logical, PIPELINES.compute.pipeline,
                    nullptr);
  vkDestroyPipelineLayout(MECHANICS.mainDevice.logical,
                          PIPELINES.compute.pipelineLayout, nullptr);

  vkDestroyRenderPass(MECHANICS.mainDevice.logical, RENDER_CONFIG.renderPass,
                      nullptr);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyBuffer(MECHANICS.mainDevice.logical,
                    MEM_COMMANDS.uniformBuffers[i], nullptr);
    vkFreeMemory(MECHANICS.mainDevice.logical,
                 MEM_COMMANDS.uniformBuffersMemory[i], nullptr);
  }

  vkDestroyDescriptorPool(MECHANICS.mainDevice.logical,
                          MEM_COMMANDS.descriptorPool, nullptr);

  vkDestroyDescriptorSetLayout(MECHANICS.mainDevice.logical,
                               MEM_COMMANDS.descriptorSetLayout, nullptr);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyBuffer(MECHANICS.mainDevice.logical,
                    MEM_COMMANDS.shaderStorageBuffers[i], nullptr);
    vkFreeMemory(MECHANICS.mainDevice.logical,
                 MEM_COMMANDS.shaderStorageBuffersMemory[i], nullptr);
  }

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(MECHANICS.mainDevice.logical,
                       MECHANICS.renderFinishedSemaphores[i], nullptr);
    vkDestroySemaphore(MECHANICS.mainDevice.logical,
                       MECHANICS.imageAvailableSemaphores[i], nullptr);
    vkDestroySemaphore(MECHANICS.mainDevice.logical,
                       MECHANICS.computeFinishedSemaphores[i], nullptr);
    vkDestroyFence(MECHANICS.mainDevice.logical, MECHANICS.inFlightFences[i],
                   nullptr);
    vkDestroyFence(MECHANICS.mainDevice.logical,
                   MECHANICS.computeInFlightFences[i], nullptr);
  }

  vkDestroyCommandPool(MECHANICS.mainDevice.logical, MEM_COMMANDS.commandPool,
                       nullptr);

  vkDestroyDevice(MECHANICS.mainDevice.logical, nullptr);

  if (LOG_LAYERS.enableValidationLayers) {
    LOG_LAYERS.DestroyDebugUtilsMessengerEXT(
        MECHANICS.instance, LOG_LAYERS.debugMessenger, nullptr);
  }

  vkDestroySurfaceKHR(MECHANICS.instance, MECHANICS.surface, nullptr);
  vkDestroyInstance(MECHANICS.instance, nullptr);

  glfwDestroyWindow(WINDOW.window);

  glfwTerminate();
}

Globals::~Globals() {
  cleanup();
}
