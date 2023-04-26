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
  while (!glfwWindowShouldClose(Globals::obj.mainWindow.window)) {
    glfwPollEvents();
    drawFrame();
    Globals::obj.mainWindow.mouseClick(Globals::obj.mainWindow.window,
                                       GLFW_MOUSE_BUTTON_LEFT);
  }
  LOG("{ main }", "terminated");
}

void CapitalEngine::initVulkan() {
  LOG("{ ** }", "initializing Capital Engine");

  // Init Vulkan
  mechanics.createInstance();
  Globals::obj.debug.setupDebugMessenger(mechanics.instance);
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
  pipelines.createGraphicsPipeline();
  pipelines.createComputePipeline();

  renderConfig.createFrameBuffers();

  memCommands.createCommandPool();
  memCommands.createShaderStorageBuffers();
  memCommands.createUniformBuffers();

  memCommands.createDescriptorPool();
  memCommands.createComputeDescriptorSets();

  memCommands.createCommandBuffers();
  memCommands.createComputeCommandBuffers();

  mechanics.createSyncObjects();
}

void CapitalEngine::drawFrame() {
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  // Compute submission
  vkWaitForFences(mechanics.mainDevice.logical, 1,
                  &mechanics.computeInFlightFences[mechanics.currentFrame],
                  VK_TRUE, UINT64_MAX);

  memCommands.updateUniformBuffer(mechanics.currentFrame);

  vkResetFences(mechanics.mainDevice.logical, 1,
                &mechanics.computeInFlightFences[mechanics.currentFrame]);

  vkResetCommandBuffer(
      memCommands.computeCommandBuffers[mechanics.currentFrame],
      /*VkCommandBufferResetFlagBits*/ 0);
  memCommands.recordComputeCommandBuffer(
      memCommands.computeCommandBuffers[mechanics.currentFrame]);

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers =
      &memCommands.computeCommandBuffers[mechanics.currentFrame];
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores =
      &mechanics.computeFinishedSemaphores[mechanics.currentFrame];

  if (vkQueueSubmit(mechanics.queues.compute, 1, &submitInfo,
                    mechanics.computeInFlightFences[mechanics.currentFrame]) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to submit compute command buffer!");
  };

  // Graphics submission
  vkWaitForFences(mechanics.mainDevice.logical, 1,
                  &mechanics.inFlightFences[mechanics.currentFrame], VK_TRUE,
                  UINT64_MAX);

  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(
      mechanics.mainDevice.logical, mechanics.swapChain, UINT64_MAX,
      mechanics.imageAvailableSemaphores[mechanics.currentFrame],
      VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    mechanics.recreateSwapChain();
    return;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  vkResetFences(mechanics.mainDevice.logical, 1,
                &mechanics.inFlightFences[mechanics.currentFrame]);

  vkResetCommandBuffer(memCommands.commandBuffers[mechanics.currentFrame],
                       /*VkCommandBufferResetFlagBits*/ 0);
  memCommands.recordCommandBuffer(
      memCommands.commandBuffers[mechanics.currentFrame], imageIndex);

  VkSemaphore waitSemaphores[] = {
      mechanics.computeFinishedSemaphores[mechanics.currentFrame],
      mechanics.imageAvailableSemaphores[mechanics.currentFrame]};
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
      &memCommands.commandBuffers[mechanics.currentFrame];
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores =
      &mechanics.renderFinishedSemaphores[mechanics.currentFrame];

  if (vkQueueSubmit(mechanics.queues.graphics, 1, &submitInfo,
                    mechanics.inFlightFences[mechanics.currentFrame]) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores =
      &mechanics.renderFinishedSemaphores[mechanics.currentFrame];

  VkSwapchainKHR swapChains[] = {mechanics.swapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;

  presentInfo.pImageIndices = &imageIndex;

  result = vkQueuePresentKHR(mechanics.queues.present, &presentInfo);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      Globals::obj.mainWindow.framebufferResized) {
    Globals::obj.mainWindow.framebufferResized = false;
    mechanics.recreateSwapChain();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  mechanics.currentFrame = (mechanics.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void CapitalEngine::cleanup() {
  LOG("{ cleanup }");
  mechanics.cleanupSwapChain();

  vkDestroyPipeline(mechanics.mainDevice.logical, pipelines.graphics.pipeline,
                    nullptr);
  vkDestroyPipelineLayout(mechanics.mainDevice.logical,
                          pipelines.graphics.pipelineLayout, nullptr);

  vkDestroyPipeline(mechanics.mainDevice.logical, pipelines.compute.pipeline,
                    nullptr);
  vkDestroyPipelineLayout(mechanics.mainDevice.logical,
                          pipelines.compute.pipelineLayout, nullptr);

  vkDestroyRenderPass(mechanics.mainDevice.logical, renderConfig.renderPass,
                      nullptr);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyBuffer(mechanics.mainDevice.logical, memCommands.uniformBuffers[i],
                    nullptr);
    vkFreeMemory(mechanics.mainDevice.logical,
                 memCommands.uniformBuffersMemory[i], nullptr);
  }

  vkDestroyDescriptorPool(mechanics.mainDevice.logical,
                          memCommands.descriptorPool, nullptr);

  vkDestroyDescriptorSetLayout(mechanics.mainDevice.logical,
                               memCommands.descriptorSetLayout, nullptr);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyBuffer(mechanics.mainDevice.logical,
                    memCommands.shaderStorageBuffers[i], nullptr);
    vkFreeMemory(mechanics.mainDevice.logical,
                 memCommands.shaderStorageBuffersMemory[i], nullptr);
  }

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(mechanics.mainDevice.logical,
                       mechanics.renderFinishedSemaphores[i], nullptr);
    vkDestroySemaphore(mechanics.mainDevice.logical,
                       mechanics.imageAvailableSemaphores[i], nullptr);
    vkDestroySemaphore(mechanics.mainDevice.logical,
                       mechanics.computeFinishedSemaphores[i], nullptr);
    vkDestroyFence(mechanics.mainDevice.logical, mechanics.inFlightFences[i],
                   nullptr);
    vkDestroyFence(mechanics.mainDevice.logical,
                   mechanics.computeInFlightFences[i], nullptr);
  }

  vkDestroyCommandPool(mechanics.mainDevice.logical, memCommands.commandPool,
                       nullptr);

  vkDestroyDevice(mechanics.mainDevice.logical, nullptr);

  if (Globals::obj.debug.enableValidationLayers) {
    Globals::obj.debug.DestroyDebugUtilsMessengerEXT(
        mechanics.instance, Globals::obj.debug.debugMessenger, nullptr);
  }

  vkDestroySurfaceKHR(mechanics.instance, mechanics.surface, nullptr);
  vkDestroyInstance(mechanics.instance, nullptr);

  glfwDestroyWindow(Globals::obj.mainWindow.window);

  glfwTerminate();
}
