#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"

#include <set>
#include <stdexcept>

#include "Debug.h"
#include "Settings.h"
#include "Vulkan_Mechanics.h"
#include "Window.h"

VulkanMechanics::VulkanMechanics()
    : surface{},
      instance{},
      mainDevice{VK_NULL_HANDLE, VK_NULL_HANDLE},
      deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME} {
  LOG("... constructing Vulkan Mechanics");
}

VulkanMechanics::~VulkanMechanics() {
  LOG("... destructing Vulkan Mechanics");
}

void VulkanMechanics::createInstance() {
  LOG(".... creating Vulkan Instance");

  const std::vector<const char*> requiredExtensions = getRequiredExtensions();

  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Human Capital";
  appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  appInfo.pEngineName = "CAPITAL Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
  appInfo.apiVersion = VK_API_VERSION_1_3;

  VkInstanceCreateInfo instanceCreateInfo{};
  instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceCreateInfo.pApplicationInfo = &appInfo;

  instanceCreateInfo.enabledExtensionCount =
      static_cast<uint32_t>(requiredExtensions.size());
  instanceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
  if (debug.enableValidationLayers) {
    if (!debug.checkValidationLayerSupport()) {
      throw std::runtime_error(
          "validation layers requested, but not available!");
    }
    instanceCreateInfo.enabledLayerCount =
        static_cast<uint32_t>(debug.validationLayers.size());
    instanceCreateInfo.ppEnabledLayerNames = debug.validationLayers.data();

    debug.populateDebugMessengerCreateInfo(debugCreateInfo);
    instanceCreateInfo.pNext =
        (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
  } else {
    instanceCreateInfo.enabledLayerCount = 0;
    instanceCreateInfo.pNext = nullptr;
  }

  if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS) {
    throw std::runtime_error("failed to create instance!");
  }
}

void VulkanMechanics::createSurface() {
  LOG(".... creating Surface");

  if (glfwCreateWindowSurface(instance, mainWindow.window, nullptr, &surface) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
  }
}

void VulkanMechanics::pickPhysicalDevice() {
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  for (VkPhysicalDevice device : devices) {
    if (isDeviceSuitable(device)) {
      mainDevice.physicalDevice = device;

      VkPhysicalDeviceProperties deviceProperties;
      vkGetPhysicalDeviceProperties(device, &deviceProperties);

      LOG("---- GPU:", deviceProperties.deviceName);
      break;
    }
  }

  if (mainDevice.physicalDevice == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }
}

VulkanMechanics::QueueFamilyIndices VulkanMechanics::findQueueFamilies(
    VkPhysicalDevice physicalDevice) {
  VulkanMechanics::QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           queueFamilies.data());
  int i = 0;
  for (const auto& queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
    }
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface,
                                         &presentSupport);
    if (presentSupport) {
      indices.presentFamily = i;
    }
    if (indices.isComplete()) {
      break;
    }
    i++;
  }
  return indices;
}

bool VulkanMechanics::checkDeviceExtensionSupport(
    VkPhysicalDevice physicalDevice) {
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount,
                                       nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount,
                                       availableExtensions.data());
  std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                           deviceExtensions.end());

  for (const auto& extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }
  return requiredExtensions.empty();
}

bool VulkanMechanics::isDeviceSuitable(VkPhysicalDevice physicalDevice) {
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

  bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);

  bool swapChainAdequate = false;
  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport =
        querySwapChainSupport(physicalDevice);
    swapChainAdequate = !swapChainSupport.formats.empty() &&
                        !swapChainSupport.presentModes.empty();
  }

  return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

std::vector<const char*> VulkanMechanics::getRequiredExtensions() {
  LOG(".... acquiring Required Extensions");

  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char*> extensions(glfwExtensions,
                                      glfwExtensions + glfwExtensionCount);

  if (debug.enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}

VulkanMechanics::SwapChainSupportDetails VulkanMechanics::querySwapChainSupport(
    VkPhysicalDevice physicalDevice) {
  VulkanMechanics::SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface,
                                            &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
                                       nullptr);
  details.formats.resize(formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
                                       details.formats.data());

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
                                            &presentModeCount, nullptr);
  details.presentModes.resize(presentModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      physicalDevice, surface, &presentModeCount, details.presentModes.data());

  return details;
}

std::ostream& operator<<(std::ostream& os, VkPhysicalDeviceProperties& device) {
  // os << device.limits.maxUniformBufferRange;
  return os;
}
