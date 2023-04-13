#pragma once
#include <vulkan/vulkan.h>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#define LOG logging.console

class Logging {
 public:
  Logging();
  ~Logging();

  std::ofstream logFile;

  template <class... Ts>
  void console(Ts&&... inputs) {
    int i = 0;
    if (!logFile.is_open()) {
      std::cerr << "!!! Could not open logFile for writing !!!" << std::endl;
      return;
    }
    std::cout << returnDateAndTime();
    logFile << returnDateAndTime();
    (
        [&] {
          ++i;
          std::cerr << " " << inputs;
          logFile << " " << inputs;
        }(),
        ...);
    std::cerr << std::endl;
    logFile << std::endl;
  }

 private:
  std::string returnDateAndTime();
};
inline Logging logging{};

class ValidationLayers {
 public:
  ValidationLayers();
  ~ValidationLayers();

  VkDebugUtilsMessengerEXT debugMessenger;
  const std::vector<const char*> validationLayers;

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
    const std::string debugMessage = pCallbackData->pMessage;
    surpressError(debugMessage, "Epic Games");
    return VK_FALSE;
  }

  void static surpressError(const std::string data, std::string checkFor);

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

inline ValidationLayers debug{};
