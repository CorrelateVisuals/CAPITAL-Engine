#pragma once
#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>
#include <vector>

#define LOG logging.console

class Logging {
  // TODO:
  // - Log complete objects and varius types: int, float, glm::vec3..
  // - Write to a log file
  // - Add date and time before logging message

 public:
  template <class... Ts>
  void console(Ts&&... inputs) {
    int i = 0;
    (
        [&] {
          ++i;
          std::cerr << " " << inputs;
        }(),
        ...);
    std::cerr << std::endl;
  }
};

inline Logging logging;

class ValidationLayers {
 public:
  ValidationLayers();
  ~ValidationLayers();

  VkDebugUtilsMessengerEXT debugMessenger;

  const std::vector<const char*> validationLayers = {
      "VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif

  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
  }

  VkResult CreateDebugUtilsMessengerEXT(
      VkInstance instance,
      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
      const VkAllocationCallbacks* pAllocator,
      VkDebugUtilsMessengerEXT* pDebugMessenger);
  void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                     VkDebugUtilsMessengerEXT debugMessenger,
                                     const VkAllocationCallbacks* pAllocator);

  void populateDebugMessengerCreateInfo(
      VkDebugUtilsMessengerCreateInfoEXT& createInfo);
  void setupDebugMessenger(VkInstance instance);
  bool checkValidationLayerSupport();
};

inline ValidationLayers debug;
