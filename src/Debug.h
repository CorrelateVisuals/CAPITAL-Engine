#pragma once
#include <vulkan/vulkan.h>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

class Logging {
 public:
  Logging();
  ~Logging();

  std::ofstream logFile;

  std::string previousTime = "";

  template <class... Ts>
  void console(Ts&&... inputs) {
    int i = 0;
    if (!logFile.is_open()) {
      std::cerr << "!!! Could not open logFile for writing !!!" << std::endl;
      return;
    }
    std::string currentTime = returnDateAndTime();

    if (currentTime != previousTime) {
      std::cout << returnDateAndTime();
      logFile << returnDateAndTime();
    } else {
      std::cout << "                 ";
    }
    (
        [&] {
          ++i;
          std::cerr << " " << inputs;
          logFile << " " << inputs;
        }(),
        ...);
    std::cerr << std::endl;
    logFile << std::endl;
    previousTime = currentTime;
  }

 private:
  std::string returnDateAndTime();
};

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
    logValidationMessage(debugMessage, "Epic Games");
    return VK_FALSE;
  }

  void setupDebugMessenger(VkInstance instance);
  void populateDebugMessengerCreateInfo(
      VkDebugUtilsMessengerCreateInfoEXT& createInfo);
  bool checkValidationLayerSupport();
  void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                     VkDebugUtilsMessengerEXT debugMessenger,
                                     const VkAllocationCallbacks* pAllocator);

 private:
  void static logValidationMessage(const std::string& string,
                                   const std::string& excludeError);
  VkResult CreateDebugUtilsMessengerEXT(
      VkInstance instance,
      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
      const VkAllocationCallbacks* pAllocator,
      VkDebugUtilsMessengerEXT* pDebugMessenger);
};
