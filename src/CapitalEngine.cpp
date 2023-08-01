#include <iostream>

#include "CapitalEngine.h"
#include "Debug.h"
#include "Mechanics.h"
#include "Memory.h"
#include "Pipelines.h"
#include "Window.h"

CapitalEngine::CapitalEngine() {
  _log.console("\n", _log.style.indentSize, "[ CAPITAL engine ]",
               "starting...\n");

  compileShaders();
  initVulkan();
}

CapitalEngine::~CapitalEngine() {
  _log.console("\n", _log.style.indentSize, "[ CAPITAL engine ]",
               "terminating...\n");
}

Global::~Global() {
  cleanup();
}

void CapitalEngine::mainLoop() {
  _log.console("\n", _log.style.indentSize,
               "{ Main Loop } running ..........\n");

  while (!glfwWindowShouldClose(_window.window)) {
    glfwPollEvents();

    _window.setMouse();
    _control.setPassedHours();

    drawFrame();

    if (glfwGetKey(_window.window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      break;
    }
  }
  vkDeviceWaitIdle(_mechanics.mainDevice.logical);
  _log.console("\n", _log.style.indentSize, "{ Main Loop } ....... terminated");
}

void CapitalEngine::compileShaders() {
  _log.console("{ SHA }", "compiling shaders");
#ifdef _WIN32
  // auto err = std::system("cmd /C \"..\\shaders\\compile_shaders.bat >
  // NUL\"");
  auto err = std::system("..\\shaders\\compile_shaders.bat");

#else
  // Linux-specific code
  auto err = std::system("./shaders/compile_shaders.sh");
#endif
}

void CapitalEngine::initVulkan() {
  _log.console("{ *** }", "initializing Capital Engine");
  _mechanics.createInstance();
  _validation.setupDebugMessenger(_mechanics.instance);
  _mechanics.createSurface();
  _mechanics.pickPhysicalDevice();
  _mechanics.createLogicalDevice();
  _mechanics.createSwapChain();
  _mechanics.createImageViews();

  _pipelines.createRenderPass();
  _memory.createDescriptorSetLayout();
  _pipelines.createGraphicsPipeline();
  _pipelines.createComputePipeline();

  _memory.createCommandPool();
  _pipelines.createColorResources();
  _pipelines.createDepthResources();
  _memory.createFramebuffers();

  _memory.createShaderStorageBuffers();
  _memory.createUniformBuffers();
  _memory.createDescriptorPool();
  _memory.createDescriptorSets();

  _memory.createCommandBuffers();
  _memory.createComputeCommandBuffers();

  _mechanics.createSyncObjects();
}

void CapitalEngine::drawFrame() {
  // Compute submission
  vkWaitForFences(
      _mechanics.mainDevice.logical, 1,
      &_mechanics.syncObjects
           .computeInFlightFences[_mechanics.syncObjects.currentFrame],
      VK_TRUE, UINT64_MAX);

  _memory.updateUniformBuffer(_mechanics.syncObjects.currentFrame);

  vkResetFences(
      _mechanics.mainDevice.logical, 1,
      &_mechanics.syncObjects
           .computeInFlightFences[_mechanics.syncObjects.currentFrame]);

  vkResetCommandBuffer(
      _memory.buffers.command.compute[_mechanics.syncObjects.currentFrame], 0);
  _memory.recordComputeCommandBuffer(
      _memory.buffers.command.compute[_mechanics.syncObjects.currentFrame]);

  VkSubmitInfo computeSubmitInfo{
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .commandBufferCount = 1,
      .pCommandBuffers =
          &_memory.buffers.command.compute[_mechanics.syncObjects.currentFrame],
      .signalSemaphoreCount = 1,
      .pSignalSemaphores =
          &_mechanics.syncObjects
               .computeFinishedSemaphores[_mechanics.syncObjects.currentFrame]};

  _mechanics.result(
      vkQueueSubmit, _mechanics.queues.compute, 1, &computeSubmitInfo,
      _mechanics.syncObjects
          .computeInFlightFences[_mechanics.syncObjects.currentFrame]);

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
    throw std::runtime_error("\n!ERROR! failed to acquire swap chain image!");
  }

  vkResetFences(_mechanics.mainDevice.logical, 1,
                &_mechanics.syncObjects
                     .inFlightFences[_mechanics.syncObjects.currentFrame]);

  vkResetCommandBuffer(
      _memory.buffers.command.graphic[_mechanics.syncObjects.currentFrame], 0);

  _memory.recordCommandBuffer(
      _memory.buffers.command.graphic[_mechanics.syncObjects.currentFrame],
      imageIndex);

  std::vector<VkSemaphore> waitSemaphores{
      _mechanics.syncObjects
          .computeFinishedSemaphores[_mechanics.syncObjects.currentFrame],
      _mechanics.syncObjects
          .imageAvailableSemaphores[_mechanics.syncObjects.currentFrame]};
  std::vector<VkPipelineStageFlags> waitStages{
      VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

  VkSubmitInfo graphicsSubmitInfo{
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
      .pWaitSemaphores = waitSemaphores.data(),
      .pWaitDstStageMask = waitStages.data(),
      .commandBufferCount = 1,
      .pCommandBuffers =
          &_memory.buffers.command.graphic[_mechanics.syncObjects.currentFrame],
      .signalSemaphoreCount = 1,
      .pSignalSemaphores =
          &_mechanics.syncObjects
               .renderFinishedSemaphores[_mechanics.syncObjects.currentFrame]};

  _mechanics.result(vkQueueSubmit, _mechanics.queues.graphics, 1,
                    &graphicsSubmitInfo,
                    _mechanics.syncObjects
                        .inFlightFences[_mechanics.syncObjects.currentFrame]);

  std::vector<VkSwapchainKHR> swapChains{_mechanics.swapChain.swapChain};

  VkPresentInfoKHR presentInfo{
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores =
          &_mechanics.syncObjects
               .renderFinishedSemaphores[_mechanics.syncObjects.currentFrame],
      .swapchainCount = 1,
      .pSwapchains = swapChains.data(),
      .pImageIndices = &imageIndex};

  result = vkQueuePresentKHR(_mechanics.queues.present, &presentInfo);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      _window.framebufferResized) {
    _window.framebufferResized = false;
    _mechanics.recreateSwapChain();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("\n!ERROR! failed to present swap chain image!");
  }

  _mechanics.syncObjects.currentFrame =
      (_mechanics.syncObjects.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Global::cleanup() {
  _mechanics.cleanupSwapChain();

  vkDestroyPipeline(_mechanics.mainDevice.logical, _pipelines.graphics.pipeline,
                    nullptr);
  vkDestroyPipelineLayout(_mechanics.mainDevice.logical,
                          _pipelines.graphics.pipelineLayout, nullptr);

  vkDestroyPipeline(_mechanics.mainDevice.logical, _pipelines.compute.pipeline,
                    nullptr);
  vkDestroyPipelineLayout(_mechanics.mainDevice.logical,
                          _pipelines.compute.pipelineLayout, nullptr);

  vkDestroyRenderPass(_mechanics.mainDevice.logical,
                      _pipelines.graphics.renderPass, nullptr);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyBuffer(_mechanics.mainDevice.logical,

                    _memory.buffers.uniforms[i], nullptr);
    vkFreeMemory(_mechanics.mainDevice.logical,
                 _memory.buffers.uniformsMemory[i], nullptr);
  }

  vkDestroyDescriptorPool(_mechanics.mainDevice.logical,
                          _memory.descriptor.pool, nullptr);

  vkDestroyDescriptorSetLayout(_mechanics.mainDevice.logical,
                               _memory.descriptor.setLayout, nullptr);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyBuffer(_mechanics.mainDevice.logical,
                    _memory.buffers.shaderStorage[i], nullptr);
    vkFreeMemory(_mechanics.mainDevice.logical,
                 _memory.buffers.shaderStorageMemory[i], nullptr);
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

  vkDestroyCommandPool(_mechanics.mainDevice.logical,
                       _memory.buffers.command.pool, nullptr);

  vkDestroyDevice(_mechanics.mainDevice.logical, nullptr);

  if (_validation.enableValidationLayers) {
    _validation.DestroyDebugUtilsMessengerEXT(
        _mechanics.instance, _validation.debugMessenger, nullptr);
  }

  vkDestroySurfaceKHR(_mechanics.instance, _mechanics.surface, nullptr);
  vkDestroyInstance(_mechanics.instance, nullptr);

  glfwDestroyWindow(_window.window);

  glfwTerminate();
}
