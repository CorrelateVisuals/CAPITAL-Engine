#pragma once

#include <string>
#include <vector>

class VulkanMechanics {
 public:
  VulkanMechanics(){};
  ~VulkanMechanics(){};

  VkSurfaceKHR surface;
  VkInstance instance;

  void createInstance();
  void createSurface();

  std::vector<const char*> getRequiredExtensions();

 private:
};

inline VulkanMechanics vulkanMechanics;
