#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <vector>

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

  const std::vector<const char*> deviceExtensions;

  struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    bool isComplete() const {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  } queueFamilyIndices;

  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  } swapChainSupport;

  VkQueue graphicsQueue;
  VkQueue presentQueue;

  void createInstance();
  void createSurface();

  void pickPhysicalDevice();
  void createLogicalDevice();

 private:
  std::vector<const char*> getRequiredExtensions();
  bool isDeviceSuitable(VkPhysicalDevice physicalDevice);
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice);
  bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice);
  SwapChainSupportDetails querySwapChainSupport(
      VkPhysicalDevice physicalDevice);
};

inline VulkanMechanics vulkanMechanics;
