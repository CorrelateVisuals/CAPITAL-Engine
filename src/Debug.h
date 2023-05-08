#pragma once
#include <vulkan/vulkan.h>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

inline std::ostream& operator<<(std::ostream& os, const std::vector<int>& vec) {
  const int numColumns = 15;
  static int elementCount = 0;
  for (const auto& element : vec) {
    if (elementCount % numColumns == 0 && elementCount != 0) {
      os << "\n                      .....  ";
      elementCount = 0;
    }
    os << element << " ";
    elementCount++;
  }
  return os;
}

class Logging {
 public:
  Logging();
  ~Logging();

  std::ofstream logFile;

  std::string previousTime = "";

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
      std::cout << "                   ";
    }

    // If the first input is a vector, handle it separately
    if constexpr (std::is_same_v<T, std::vector<int>>) {
      const int numColumns = 15;
      static int elementCount = 0;
      for (const auto& element : first) {
        if (elementCount % numColumns == 0 && elementCount != 0) {
          std::cout << "\n                      .....  ";
          logFile << "\n                      .....  ";
          elementCount = 0;
        }
        std::cout << element << " ";
        logFile << element << " ";
        elementCount++;
      }
      std::cout << "\n";
      logFile << "\n";
    } else {
      // Handle all other inputs normally
      std::cerr << " " << first;
      logFile << " " << first;
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
