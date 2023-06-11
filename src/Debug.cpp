#include <chrono>
#include <ctime>
#include <iostream>
#include <set>
#include <string>

#include "CapitalEngine.h"

Logging::Logging()
    : logFile("log.txt", std::ofstream::out | std::ofstream::trunc) {
  _log.console("{ ... }", "constructing Logging");
}

Logging::~Logging() {
  _log.console("{ ... }", "destructing Logging");
}

ValidationLayers::ValidationLayers()
    : debugMessenger{}, validation{"VK_LAYER_KHRONOS_validation"} {
  _log.console("{ --- }", "constructing Validation Layers");
}

ValidationLayers::~ValidationLayers() {
  _log.console("{ --- }", "destructing Validation Layers");
}

void ValidationLayers::logValidationMessage(const std::string& string,
                                            const std::string& excludeError) {
  if (string.find(excludeError) != std::string::npos)
    return;

  _log.console("\n\n                     > > > Validation Layer: ", string,
               "\n");
}

VkResult ValidationLayers::CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void ValidationLayers::DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}

void ValidationLayers::populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
  createInfo = {
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = debugCallback};
}

void ValidationLayers::setupDebugMessenger(VkInstance instance) {
  if (!enableValidationLayers)
    return;

  VkDebugUtilsMessengerCreateInfoEXT createInfo;
  populateDebugMessengerCreateInfo(createInfo);

  if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr,
                                   &debugMessenger) != VK_SUCCESS)
    throw std::runtime_error("!ERROR! Failed to set up debug messenger!");
}

bool ValidationLayers::checkValidationLayerSupport() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  std::set<std::string> availableLayerNames;
  for (const auto& layer : availableLayers) {
    availableLayerNames.insert(layer.layerName);
  }

  for (const auto& layerName : validation) {
    if (availableLayerNames.find(layerName) == availableLayerNames.end()) {
      return false;
    }
  }
  return true;
}

std::string Logging::returnDateAndTime() {
  auto now = std::chrono::system_clock::now();
  std::time_t nowC = std::chrono::system_clock::to_time_t(now);
  std::tm timeInfo;

#ifdef __linux__
  char nowStr[20] = "---";
#elif _WIN32
  gmtime_s(&timeInfo, &nowC);
  char nowStr[20];
  strftime(nowStr, 20, "%y.%m.%d %H:%M:%S", &timeInfo);
#else

#endif

  return std::string(nowStr);
}

std::string Logging::getBufferUsageString(VkBufferUsageFlags usage) {
  std::string result;

  if (usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT) {
    result += "TRANSFER_SRC | ";
  }
  if (usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT) {
    result += "TRANSFER_DST | ";
  }
  if (usage & VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT) {
    result += "UNIFORM_TEXEL_BUFFER | ";
  }
  if (usage & VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT) {
    result += "STORAGE_TEXEL_BUFFER | ";
  }
  if (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) {
    result += "UNIFORM_BUFFER | ";
  }
  if (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
    result += "STORAGE_BUFFER | ";
  }
  if (usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT) {
    result += "INDEX_BUFFER | ";
  }
  if (usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) {
    result += "VERTEX_BUFFER | ";
  }
  if (usage & VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT) {
    result += "INDIRECT_BUFFER | ";
  }

  // Remove the trailing " | " if there is one.
  if (!result.empty()) {
    result.erase(result.length() - 3);
  }

  return result;
}
