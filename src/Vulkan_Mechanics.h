#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <vector>

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

  struct Queues {
    VkQueue graphics;
    VkQueue compute;
    VkQueue present;
  } queues;

  struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    bool isComplete() const {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  } queueFamilyIndices;

  VkSwapchainKHR swapChain;
  std::vector<VkImage> swapChainImages;
  VkFormat swapChainImageFormat;
  std::vector<VkImageView> swapChainImageViews;
  VkExtent2D swapChainExtent;
  std::vector<VkFramebuffer> swapChainFramebuffers;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkSemaphore> computeFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  std::vector<VkFence> computeInFlightFences;
  uint32_t currentFrame = 0;

  void createInstance();
  void createSurface();

  void pickPhysicalDevice();
  void createLogicalDevice();

  void createSwapChain();
  void recreateSwapChain();

  void createSyncObjects();

  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physical);

 private:
  const std::vector<const char*> deviceExtensions;

  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  } swapChainSupport;

  std::vector<const char*> getRequiredExtensions();
  bool isDeviceSuitable(VkPhysicalDevice physical);
  bool checkDeviceExtensionSupport(VkPhysicalDevice physical);
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physical);
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR>& availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR>& availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
};
inline VulkanMechanics mechanics;

class RenderConfiguration {
 public:
  RenderConfiguration();
  ~RenderConfiguration();

  VkImage depthImage;
  VkDeviceMemory depthImageMemory;
  VkImageView depthImageView;
  VkFormat depthFormat;

  VkRenderPass renderPass;

  void createImageViews();
  void createRenderPass();

  void createDepthResources();
  void createFrameBuffers();

 private:
  VkFormat findDepthFormat();
  VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
                               VkImageTiling tiling,
                               VkFormatFeatureFlags features);
  bool hasStencilComponent(VkFormat format);
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
