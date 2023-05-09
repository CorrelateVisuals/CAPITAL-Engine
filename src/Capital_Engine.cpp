#include <iostream>

#include "CAPITAL_Engine.h"
#include "Debug.h"
#include "Mechanics.h"
#include "Pipelines.h"
#include "Window.h"

CapitalEngine::CapitalEngine() {
  _log.console("\n                   [", "Starting CAPITAL engine", "]\n");

  initVulkan();
}

CapitalEngine::~CapitalEngine() {
  _log.console("\n                   [", "Terminating CAPITAL engine", "]\n");
}

Global::~Global() {
  cleanup();
}

void CapitalEngine::mainLoop() {
  _log.console("\n                  ", "main loop running ..........\n");
  while (!glfwWindowShouldClose(_window.window)) {
    glfwPollEvents();

    _control.simulateHours();
    drawFrame();

    _window.mouseClick(_window.window, GLFW_MOUSE_BUTTON_LEFT);
  }
  vkDeviceWaitIdle(_mechanics.mainDevice.logical);
  _log.console("\n                  ", "main loop", "....... terminated");
}

void CapitalEngine::initVulkan() {
  _log.console("{ *** }", "initializing Capital Engine");
  _mechanics.createInstance();
  _validationLayers.setupDebugMessenger(_mechanics.instance);
  _mechanics.createSurface();
  _mechanics.pickPhysicalDevice();
  _mechanics.createLogicalDevice();
  _mechanics.createSwapChain();
  _mechanics.createImageViews();

  _pipelines.createRenderPass();
  _memCommands.createComputeDescriptorSetLayout();
  _pipelines.createGraphicsPipeline();
  _pipelines.createComputePipeline();

  _memCommands.createFramebuffers();
  _memCommands.createCommandPool();
  _memCommands.createShaderStorageBuffers();

  _memCommands.createUniformBuffers();
  _memCommands.createDescriptorPool();
  _memCommands.createComputeDescriptorSets();

  _memCommands.createCommandBuffers();
  _memCommands.createComputeCommandBuffers();

  _mechanics.createSyncObjects();
}

void CapitalEngine::drawFrame() {
  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  // Compute submission
  vkWaitForFences(
      _mechanics.mainDevice.logical, 1,
      &_mechanics.syncObjects
           .computeInFlightFences[_mechanics.syncObjects.currentFrame],
      VK_TRUE, UINT64_MAX);

  _memCommands.updateUniformBuffer(_mechanics.syncObjects.currentFrame);

  vkResetFences(
      _mechanics.mainDevice.logical, 1,
      &_mechanics.syncObjects
           .computeInFlightFences[_mechanics.syncObjects.currentFrame]);

  vkResetCommandBuffer(
      _memCommands.command.computeBuffers[_mechanics.syncObjects.currentFrame],
      /*VkCommandBufferResetFlagBits*/ 0);
  _memCommands.recordComputeCommandBuffer(
      _memCommands.command.computeBuffers[_mechanics.syncObjects.currentFrame]);

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers =
      &_memCommands.command.computeBuffers[_mechanics.syncObjects.currentFrame];
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores =
      &_mechanics.syncObjects
           .computeFinishedSemaphores[_mechanics.syncObjects.currentFrame];

  if (vkQueueSubmit(
          _mechanics.queues.compute, 1, &submitInfo,
          _mechanics.syncObjects
              .computeInFlightFences[_mechanics.syncObjects.currentFrame]) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to submit compute command buffer!");
  };

  // Graphics submission
  vkWaitForFences(_mechanics.mainDevice.logical, 1,
                  &_mechanics.syncObjects
                       .inFlightFences[_mechanics.syncObjects.currentFrame],
                  VK_TRUE, UINT64_MAX);

  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(
      _mechanics.mainDevice.logical, _mechanics.swapChain.swapChain, UINT64_MAX,
      _mechanics.syncObjects
          .imageAvailableSemaphores[_mechanics.syncObjects.currentFrame],
      VK_NULL_HANDLE, &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    _mechanics.recreateSwapChain();
    return;
  } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  vkResetFences(_mechanics.mainDevice.logical, 1,
                &_mechanics.syncObjects
                     .inFlightFences[_mechanics.syncObjects.currentFrame]);

  vkResetCommandBuffer(
      _memCommands.command.graphicBuffers[_mechanics.syncObjects.currentFrame],
      /*VkCommandBufferResetFlagBits*/ 0);
  _memCommands.recordCommandBuffer(
      _memCommands.command.graphicBuffers[_mechanics.syncObjects.currentFrame],
      imageIndex);

  VkSemaphore waitSemaphores[] = {
      _mechanics.syncObjects
          .computeFinishedSemaphores[_mechanics.syncObjects.currentFrame],
      _mechanics.syncObjects
          .imageAvailableSemaphores[_mechanics.syncObjects.currentFrame]};
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
      &_memCommands.command.graphicBuffers[_mechanics.syncObjects.currentFrame];
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores =
      &_mechanics.syncObjects
           .renderFinishedSemaphores[_mechanics.syncObjects.currentFrame];

  if (vkQueueSubmit(_mechanics.queues.graphics, 1, &submitInfo,
                    _mechanics.syncObjects
                        .inFlightFences[_mechanics.syncObjects.currentFrame]) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  VkPresentInfoKHR presentInfo{};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores =
      &_mechanics.syncObjects
           .renderFinishedSemaphores[_mechanics.syncObjects.currentFrame];

  VkSwapchainKHR swapChains[] = {_mechanics.swapChain.swapChain};
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

  _mechanics.syncObjects.currentFrame =
      (_mechanics.syncObjects.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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

  vkDestroyRenderPass(_mechanics.mainDevice.logical, _pipelines.renderPass,
                      nullptr);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyBuffer(_mechanics.mainDevice.logical,
                    _memCommands.uniform.buffers[i], nullptr);
    vkFreeMemory(_mechanics.mainDevice.logical,
                 _memCommands.uniform.buffersMemory[i], nullptr);
  }

  vkDestroyDescriptorPool(_mechanics.mainDevice.logical,
                          _memCommands.descriptor.pool, nullptr);

  vkDestroyDescriptorSetLayout(_mechanics.mainDevice.logical,
                               _memCommands.descriptor.setLayout, nullptr);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyBuffer(_mechanics.mainDevice.logical,
                    _memCommands.shaderStorage.buffers[i], nullptr);
    vkFreeMemory(_mechanics.mainDevice.logical,
                 _memCommands.shaderStorage.buffersMemory[i], nullptr);
  }

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(_mechanics.mainDevice.logical,
                       _mechanics.syncObjects.renderFinishedSemaphores[i],
                       nullptr);
    vkDestroySemaphore(_mechanics.mainDevice.logical,
                       _mechanics.syncObjects.imageAvailableSemaphores[i],
                       nullptr);
    vkDestroySemaphore(_mechanics.mainDevice.logical,
                       _mechanics.syncObjects.computeFinishedSemaphores[i],
                       nullptr);
    vkDestroyFence(_mechanics.mainDevice.logical,
                   _mechanics.syncObjects.inFlightFences[i], nullptr);
    vkDestroyFence(_mechanics.mainDevice.logical,
                   _mechanics.syncObjects.computeInFlightFences[i], nullptr);
  }

  vkDestroyCommandPool(_mechanics.mainDevice.logical, _memCommands.command.pool,
                       nullptr);

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
