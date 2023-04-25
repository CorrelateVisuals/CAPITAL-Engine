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
    drawFrame();
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
      mainWindow.framebufferResized) {
    mainWindow.framebufferResized = false;
    mechanics.recreateSwapChain();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }

  mechanics.currentFrame = (mechanics.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
