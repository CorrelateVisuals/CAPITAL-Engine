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

  struct Style {
    std::string charLeader = std::string(8, ' ') + ":";
    std::string indentSize = std::string(17, ' ');
    int columnCount = 14;
  } style;

  template <class T, class... Ts>
  void console(const T& first, const Ts&... inputs);
  std::string getBufferUsageString(VkBufferUsageFlags usage);

 private:
  std::ofstream logFile;
  std::string previousTime;
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

  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void* pUserData) {
    const std::string debugMessage = pCallbackData->pMessage;
    logValidationMessage(debugMessage, "Epic Games");
    return VK_FALSE;
  }
};

template <class T, class... Ts>
void Logging::console(const T& first, const Ts&... inputs) {
  if (!logFile.is_open()) {
    std::cerr << "!ERROR! Could not open logFile for writing" << std::endl;
    return;
  }
  std::string currentTime = returnDateAndTime();
  int columnCountOffset = 4;

  if (currentTime != previousTime) {
    std::cout << ' ' << currentTime;
    logFile << ' ' << currentTime;
  } else {
    std::string padding(
        static_cast<size_t>(style.columnCount) + columnCountOffset, ' ');
    std::cout << padding;
    logFile << padding;
  }
  if constexpr (std::is_same_v<T, std::vector<int>>) {
    static int elementCount = 0;
    std::cout << ' ' << style.charLeader << ' ';
    logFile << ' ' << style.charLeader << ' ';
    for (const auto& element : first) {
      if (elementCount % style.columnCount == 0 && elementCount != 0) {
        std::string spaces(
            static_cast<size_t>(style.columnCount) + columnCountOffset, ' ');

        std::cout << '\n' << ' ' << spaces << style.charLeader << ' ';
        logFile << '\n' << ' ' << spaces << style.charLeader << ' ';

        elementCount = 0;
      }
      std::cout << element << ' ';
      logFile << element << ' ';
      elementCount++;
    }
    std::cout << '\n';
    logFile << '\n';
  } else {
    std::cout << ' ' << first;
    logFile << ' ' << first;
    ((std::cout << ' ' << inputs, logFile << ' ' << inputs), ...);
    std::cout << '\n';
    logFile << '\n';
  }
  previousTime = currentTime;
}
