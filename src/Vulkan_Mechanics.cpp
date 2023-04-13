#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"

#include <stdexcept>

#include "Debug.h"
#include "Settings.h"
#include "Vulkan_Mechanics.h"
#include "Window.h"

VulkanMechanics::VulkanMechanics() : surface{}, instance{} {
  LOG("... constructing Vulkan Mechanics");
}

VulkanMechanics::~VulkanMechanics() {
  LOG("... destructing Vulkan Mechanics");
}

void VulkanMechanics::createInstance() {
  LOG(".... creating Vulkan Instance");

  if (debug.enableValidationLayers && !debug.checkValidationLayerSupport()) {
    throw std::runtime_error("validation layers requested, but not available!");
  }

  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Human Capital";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "Capital Vulkan Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_3;

  VkInstanceCreateInfo instanceCreateInfo{};
  instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceCreateInfo.pApplicationInfo = &appInfo;

  auto extensions = getRequiredExtensions();
  instanceCreateInfo.enabledExtensionCount =
      static_cast<uint32_t>(extensions.size());
  instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
  if (debug.enableValidationLayers) {
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

// void VulkanMechanics::pickPhysicalDevice() {
//   uint32_t deviceCount = 0;
//   vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
//
//   if (deviceCount == 0) {
//     throw std::runtime_error("failed to find GPUs with Vulkan support!");
//   }
//
//   std::vector<VkPhysicalDevice> devices(deviceCount);
//   vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
//
//   for (const auto& device : devices) {
//     if (isDeviceSuitable(device)) {
//       mainDevice.physicalDevice = device;
//       break;
//     }
//   }
//
//   if (mainDevice.physicalDevice == VK_NULL_HANDLE) {
//     throw std::runtime_error("failed to find a suitable GPU!");
//   }
// }
//
// void VulkanMechanics::createLogicalDevice() {}
//
// bool VulkanMechanics::isDeviceSuitable(VkPhysicalDevice physicalDevice) {
//   VulkanMechanics::QueueFamilyIndices indices =
//       findQueueFamilies(physicalDevice);
//
//   bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);
//
//   bool swapChainAdequate = false;
//   if (extensionsSupported) {
//     VulkanMechanics::SwapChainSupportDetails swapChainSupport =
//         querySwapChainSupport(physicalDevice);
//     swapChainAdequate = !swapChainSupport.formats.empty() &&
//                         !swapChainSupport.presentModes.empty();
//   }
//
//   return indices.isComplete() && extensionsSupported && swapChainAdequate;
// }

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
