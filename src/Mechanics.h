#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "CapitalEngine.h"

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

class VulkanMechanics {
 public:
  VulkanMechanics();
  ~VulkanMechanics();

  VkSurfaceKHR surface;
  VkInstance instance;

  struct Device {
    VkPhysicalDevice physical;
    VkDevice logical;
  } mainDevice;

  const std::vector<const char*> deviceExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  struct Queues {
    VkQueue graphics;
    VkQueue compute;
    VkQueue present;

    struct FamilyIndices {
      std::optional<uint32_t> graphicsAndComputeFamily;
      std::optional<uint32_t> presentFamily;
      bool isComplete() const {
        return graphicsAndComputeFamily.has_value() &&
               presentFamily.has_value();
      }
    } familyIndices;
  } queues;

  struct SwapChain {
    VkSwapchainKHR swapChain;
    std::vector<VkImage> images;
    VkFormat imageFormat;
    std::vector<VkImageView> imageViews;
    VkExtent2D extent;
    std::vector<VkFramebuffer> framebuffers;

    struct SupportDetails {
      VkSurfaceCapabilitiesKHR capabilities{};
      std::vector<VkSurfaceFormatKHR> formats;
      std::vector<VkPresentModeKHR> presentModes;
    } supportDetails;
  } swapChain;

  struct SynchronizationObjects {
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkSemaphore> computeFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> computeInFlightFences;
    uint32_t currentFrame = 0;
  } syncObjects;

 public:
  void createInstance();
  void createSurface();

  void pickPhysicalDevice();
  void createLogicalDevice();

  void createSwapChain();
  void recreateSwapChain();
  void cleanupSwapChain();

  void createImageViews();
  VkImageView createImageView(VkImage image,
                              VkFormat format,
                              VkImageAspectFlags aspectFlags);

  void createSyncObjects();

  Queues::FamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice);

  template <typename CheckVulkanResult, typename... Args>
  void vulkanResult(CheckVulkanResult vkResult, Args&&... args);

 private:
  std::vector<const char*> getRequiredExtensions();

  bool isDeviceSuitable(VkPhysicalDevice physicalDevice);
  bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice);

  SwapChain::SupportDetails querySwapChainSupport(
      VkPhysicalDevice physicalDevice);
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR>& availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR>& availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
};

template <typename CheckVulkanResult, typename... Args>
inline void VulkanMechanics::vulkanResult(CheckVulkanResult vkResult,
                                          Args&&... args) {
  using ObjectType = std::remove_pointer_t<std::decay_t<CheckVulkanResult>>;
  std::string objectName = typeid(ObjectType).name();

  VkResult result = vkResult(std::forward<Args>(args)...);
  if (result != VK_SUCCESS) {
    throw std::runtime_error("!ERROR! result != VK_SUCCESS " + objectName +
                             "!");
  }
}
