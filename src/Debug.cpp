#include <chrono>
#include <ctime>
#include <iostream>
#include <set>
#include <string>

#include "Debug.h"

Logging::Logging()
    : logFile("log.txt", std::ofstream::out | std::ofstream::trunc) {
  LOG(". constructing Logging");
}

Logging::~Logging() {
  LOG(". destructing Logging");
}

ValidationLayers::ValidationLayers()
    : debugMessenger{}, validationLayers{"VK_LAYER_KHRONOS_validation"} {
  LOG(".. constructing Validation Layers");
}

ValidationLayers::~ValidationLayers() {
  LOG(".. destructing Validation Layers");
}

void ValidationLayers::logValidationMessage(const std::string& string,
                                            const std::string& excludeError) {
  if (string.find(excludeError) != std::string::npos)
    return;

  LOG("\n\n                     > > > Validation Layer: ", string, "\n");
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
    throw std::runtime_error("Failed to set up debug messenger!");
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

  for (const auto& layerName : validationLayers) {
    if (availableLayerNames.find(layerName) == availableLayerNames.end()) {
      return false;
    }
  }
  return true;
}

std::string Logging::returnDateAndTime() {
  auto now = std::chrono::system_clock::now();
  std::time_t now_c = std::chrono::system_clock::to_time_t(now);
  struct tm timeinfo;
  localtime_s(&timeinfo, &now_c);
  char now_str[20];
  strftime(now_str, 20, "%Y-%m-%d %H:%M:%S", &timeinfo);
  return std::string(now_str);
}
