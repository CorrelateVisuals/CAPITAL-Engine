#pragma once

#include <optional>
#include <string>
#include <vector>

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
  MainDevice mainDevice{};

  const std::vector<const char*> deviceExtensions;

  struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    bool isComplete() {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  };

  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  void createInstance();
  void createSurface();

  void pickPhysicalDevice();

 private:
  std::vector<const char*> getRequiredExtensions();
  bool isDeviceSuitable(VkPhysicalDevice physicalDevice);
  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice);
  bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice);
  SwapChainSupportDetails querySwapChainSupport(
      VkPhysicalDevice physicalDevice);
};

inline VulkanMechanics vulkanMechanics{};
