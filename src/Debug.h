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
    std::string charLeader = "        :";
    std::string indentSize = "                 ";
    static constexpr int numColumns = 14;
  } style;

  template <class T, class... Ts>
  void console(const T& first, const Ts&... inputs);
  std::string getBufferUsageString(VkBufferUsageFlags usage);

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

template <class T, class... Ts>
inline void Logging::console(const T& first, const Ts&... inputs) {
  int i = 0;
  if (!logFile.is_open()) {
    std::cerr << "!!! Could not open logFile for writing !!!" << std::endl;
    return;
  }

  std::string currentTime = returnDateAndTime();
  int numColumnsOffset = 4;
  if (currentTime != previousTime) {
    std::cout << " " << returnDateAndTime();
    logFile << " " << returnDateAndTime();
  } else {
    std::cout << std::string(
        static_cast<size_t>(style.numColumns) + numColumnsOffset, ' ');
  }

  // If the first input is a vector, handle it separately
  if constexpr (std::is_same_v<T, std::vector<int>>) {
    static int elementCount = 0;
    std::cout << " " << style.charLeader << " ";
    logFile << " " << style.charLeader << " ";
    for (const auto& element : first) {
      if (elementCount % style.numColumns == 0 && elementCount != 0) {
        std::cout << "\n "
                  << std::string(static_cast<size_t>(style.numColumns) +
                                     numColumnsOffset,
                                 ' ')
                  << style.charLeader << " ";
        logFile << "\n "
                << std::string(
                       static_cast<size_t>(style.numColumns) + numColumnsOffset,
                       ' ')
                << style.charLeader << " ";
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
