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

  static constexpr int numColumns = 15;

  template <class T, class... Ts>
  void console(const T& first, const Ts&... inputs) {
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
      std::cout << std::string(numColumns + 3, ' ');
    }

    // If the first input is a vector, handle it separately
    if constexpr (std::is_same_v<T, std::vector<int>>) {
      static int elementCount = 0;
      std::cout << "  .....  ";
      logFile << "  .....  ";
      for (const auto& element : first) {
        if (elementCount % numColumns == 0 && elementCount != 0) {
          std::cout << "\n" << std::string(numColumns + 3, ' ') << "  .....  ";
          logFile << "\n" << std::string(numColumns + 3, ' ') << "  .....  ";
          elementCount = 0;
        }
        std::cout << element << ' ';
        logFile << element << ' ';
        elementCount++;
      }
      std::cout << '\n';
      logFile << '\n';
    } else {
      // Handle all other inputs normally
      std::cerr << ' ' << first;
      logFile << ' ' << first;
      (
          [&] {
            ++i;
            std::cerr << ' ' << inputs;
            logFile << ' ' << inputs;
          }(),
          ...);
      std::cerr << '\n';
      logFile << '\n';
    }
    previousTime = currentTime;
  }

 private:
  std::ofstream logFile;
  std::string previousTime = "";

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
