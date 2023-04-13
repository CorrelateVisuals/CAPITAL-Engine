#pragma once

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

  void createInstance();
  void createSurface();

  // void pickPhysicalDevice();
  // void createLogicalDevice();

 private:
  std::vector<const char*> getRequiredExtensions();
  // bool isDeviceSuitable(VkPhysicalDevice physicalDevice);
};

inline VulkanMechanics vulkanMechanics{};
