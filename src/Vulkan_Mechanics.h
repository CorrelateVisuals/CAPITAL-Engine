#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <vector>

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

std::ostream& operator<<(std::ostream& os, VkPhysicalDeviceProperties& device);

class VulkanMechanics {
 public:
  VulkanMechanics();
  ~VulkanMechanics();

  VkSurfaceKHR surface;
  VkInstance instance;

  using MainDevice = struct {
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
  };
  MainDevice mainDevice;

  std::vector<VkImage> swapChainImages;
  VkFormat swapChainImageFormat;
  std::vector<VkImageView> swapChainImageViews;
  VkExtent2D swapChainExtent;

  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers;

  void createInstance();
  void createSurface();

  void pickPhysicalDevice();
  void createLogicalDevice();

  void createSwapChain();
  void createSyncObjects();

  void createCommandPool();
  void createCommandBuffers();

 private:
  const std::vector<const char*> deviceExtensions;

  struct Queues {
    VkQueue graphics;
    VkQueue compute;
    VkQueue present;
  };
  Queues queues;

  struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    bool isComplete() const {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  };
  QueueFamilyIndices queueFamilyIndices;

  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };
  SwapChainSupportDetails swapChainSupport;

  VkSwapchainKHR swapChain;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkFence> inFlightFences;

  // VkFormat depthFormat;

  std::vector<const char*> getRequiredExtensions();
  bool isDeviceSuitable(VkPhysicalDevice physicalDevice);
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice);
  bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice);
  SwapChainSupportDetails querySwapChainSupport(
      VkPhysicalDevice physicalDevice);

  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR>& availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR>& availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
};
inline VulkanMechanics vulkanMechanics;

class RenderConfiguration {
 public:
  RenderConfiguration();
  ~RenderConfiguration();

  void createDepthResources();
  void createImageViews();

  void setupRenderPass();
  void createPipelineCache();

  void setupFrameBuffer();

  VkImage depthImage;
  VkDeviceMemory depthImageMemory;
  VkImageView depthImageView;

 private:
  VkRenderPass renderPass;
  VkPipelineCache pipelineCache;
  std::vector<VkFramebuffer> frameBuffers;

  VkFormat findDepthFormat();
  VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
                               VkImageTiling tiling,
                               VkFormatFeatureFlags features);
  void createImage(uint32_t width,
                   uint32_t height,
                   VkFormat format,
                   VkImageTiling tiling,
                   VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties,
                   VkImage& image,
                   VkDeviceMemory& imageMemory);

  VkImageView createImageView(VkImage image,
                              VkFormat format,
                              VkImageAspectFlags aspectFlags);
  uint32_t findMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties);
};
inline RenderConfiguration renderConfig;
