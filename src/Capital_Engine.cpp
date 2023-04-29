#include <iostream>

#include "CAPITAL_Engine.h"
#include "Debug.h"
#include "Mechanics.h"
#include "Pipelines.h"
#include "Window.h"

CapitalEngine::CapitalEngine() {
  _log.console("\n                    [", "Starting CAPITAL engine", "]\n");

  initVulkan();
}

CapitalEngine::~CapitalEngine() {
  _log.console("\n                    [", "Terminating CAPITAL engine", "]\n");
}

Global::~Global() {
  cleanup();
}

void CapitalEngine::mainLoop() {
  _log.console("{ main }", "running ...");
  while (!glfwWindowShouldClose(_window.window)) {
    glfwPollEvents();
    drawFrame();
    // We want to animate the particle system using the last frames time to
    // get smooth, frame-rate independent animation
    double currentTime = glfwGetTime();
    _memCommands.lastFrameTime = (currentTime - _memCommands.lastTime) * 1000.0;
    _memCommands.lastTime = currentTime;

    _window.mouseClick(_window.window, GLFW_MOUSE_BUTTON_LEFT);
  }
  _log.console("{ main }", "terminated");
}

void CapitalEngine::initVulkan() {
  _log.console("{ ** }", "initializing Capital Engine");
  _mechanics.createInstance();
  _validationLayers.setupDebugMessenger(_mechanics.instance);
  _mechanics.createSurface();
  _mechanics.pickPhysicalDevice();
  _mechanics.createLogicalDevice();

  _mechanics.createSwapChain();
  _mechanics.createImageViews();

  // createImageViews();
  // createRenderPass();
  // createComputeDescriptorSetLayout();
  // createGraphicsPipeline();
  // createComputePipeline();
  // createFramebuffers();
  // createCommandPool();
  // createShaderStorageBuffers();
  // createUniformBuffers();
  // createDescriptorPool();
  // createComputeDescriptorSets();
  // createCommandBuffers();
  // createComputeCommandBuffers();
  // createSyncObjects();
}

void CapitalEngine::drawFrame() {
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  // Compute submission
  vkWaitForFences(_mechanics.mainDevice.logical, 1,
                  &_mechanics.computeInFlightFences[_mechanics.currentFrame],
                  VK_TRUE, UINT64_MAX);

  _memCommands.updateUniformBuffer(_mechanics.currentFrame);

  vkResetFences(_mechanics.mainDevice.logical, 1,
                &_mechanics.computeInFlightFences[_mechanics.currentFrame]);

  vkResetCommandBuffer(
      _memCommands.computeCommandBuffers[_mechanics.currentFrame],
      /*VkCommandBufferResetFlagBits*/ 0);
  _memCommands.recordComputeCommandBuffer(
      _memCommands.computeCommandBuffers[_mechanics.currentFrame]);

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers =
      &_memCommands.computeCommandBuffers[_mechanics.currentFrame];
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores =
      &_mechanics.computeFinishedSemaphores[_mechanics.currentFrame];

  if (vkQueueSubmit(
          _mechanics.queues.compute, 1, &submitInfo,
          _mechanics.computeInFlightFences[_mechanics.currentFrame]) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to submit compute command buffer!");
  };

  // Graphics submission
  vkWaitForFences(_mechanics.mainDevice.logical, 1,
                  &_mechanics.inFlightFences[_mechanics.currentFrame], VK_TRUE,
                  UINT64_MAX);

  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(
      _mechanics.mainDevice.logical, _mechanics.swapChain, UINT64_MAX,
      _mechanics.imageAvailableSemaphores[_mechanics.currentFrame],
      VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    _mechanics.recreateSwapChain();
    return;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  vkResetFences(_mechanics.mainDevice.logical, 1,
                &_mechanics.inFlightFences[_mechanics.currentFrame]);

  vkResetCommandBuffer(_memCommands.commandBuffers[_mechanics.currentFrame],
                       /*VkCommandBufferResetFlagBits*/ 0);
  _memCommands.recordCommandBuffer(
      _memCommands.commandBuffers[_mechanics.currentFrame], imageIndex);

  VkSemaphore waitSemaphores[] = {
      _mechanics.computeFinishedSemaphores[_mechanics.currentFrame],
      _mechanics.imageAvailableSemaphores[_mechanics.currentFrame]};
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
      &_memCommands.commandBuffers[_mechanics.currentFrame];
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores =
      &_mechanics.renderFinishedSemaphores[_mechanics.currentFrame];

  if (vkQueueSubmit(_mechanics.queues.graphics, 1, &submitInfo,
                    _mechanics.inFlightFences[_mechanics.currentFrame]) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores =
      &_mechanics.renderFinishedSemaphores[_mechanics.currentFrame];

  VkSwapchainKHR swapChains[] = {_mechanics.swapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;

  presentInfo.pImageIndices = &imageIndex;

  result = vkQueuePresentKHR(_mechanics.queues.present, &presentInfo);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      _window.framebufferResized) {
    _window.framebufferResized = false;
    _mechanics.recreateSwapChain();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  _mechanics.currentFrame =
      (_mechanics.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Global::cleanup() {
  _log.console("{ cleanup }");

  _mechanics.cleanupSwapChain();

  vkDestroyPipeline(_mechanics.mainDevice.logical, _pipelines.graphics.pipeline,
                    nullptr);
  vkDestroyPipelineLayout(_mechanics.mainDevice.logical,
                          _pipelines.graphics.pipelineLayout, nullptr);

  vkDestroyPipeline(_mechanics.mainDevice.logical, _pipelines.compute.pipeline,
                    nullptr);
  vkDestroyPipelineLayout(_mechanics.mainDevice.logical,
                          _pipelines.compute.pipelineLayout, nullptr);

  vkDestroyRenderPass(_mechanics.mainDevice.logical, _renderConfig.renderPass,
                      nullptr);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyBuffer(_mechanics.mainDevice.logical,
                    _memCommands.uniformBuffers[i], nullptr);
    vkFreeMemory(_mechanics.mainDevice.logical,
                 _memCommands.uniformBuffersMemory[i], nullptr);
  }

  vkDestroyDescriptorPool(_mechanics.mainDevice.logical,
                          _memCommands.descriptorPool, nullptr);

  vkDestroyDescriptorSetLayout(_mechanics.mainDevice.logical,
                               _memCommands.descriptorSetLayout, nullptr);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyBuffer(_mechanics.mainDevice.logical,
                    _memCommands.shaderStorageBuffers[i], nullptr);
    vkFreeMemory(_mechanics.mainDevice.logical,
                 _memCommands.shaderStorageBuffersMemory[i], nullptr);
  }

  vkDestroyCommandPool(_mechanics.mainDevice.logical, _memCommands.commandPool,
                       nullptr);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(_mechanics.mainDevice.logical,
                       _mechanics.renderFinishedSemaphores[i], nullptr);
    vkDestroySemaphore(_mechanics.mainDevice.logical,
                       _mechanics.imageAvailableSemaphores[i], nullptr);
    vkDestroySemaphore(_mechanics.mainDevice.logical,
                       _mechanics.computeFinishedSemaphores[i], nullptr);
    vkDestroyFence(_mechanics.mainDevice.logical, _mechanics.inFlightFences[i],
                   nullptr);
    vkDestroyFence(_mechanics.mainDevice.logical,
                   _mechanics.computeInFlightFences[i], nullptr);
  }

  vkDestroyDevice(_mechanics.mainDevice.logical, nullptr);

  if (_validationLayers.enableValidationLayers) {
    _validationLayers.DestroyDebugUtilsMessengerEXT(
        _mechanics.instance, _validationLayers.debugMessenger, nullptr);
  }

  vkDestroySurfaceKHR(_mechanics.instance, _mechanics.surface, nullptr);
  vkDestroyInstance(_mechanics.instance, nullptr);

  glfwDestroyWindow(_window.window);

  glfwTerminate();
}
