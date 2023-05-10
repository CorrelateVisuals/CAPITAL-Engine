#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"

#include <algorithm>
#include <set>
#include <stdexcept>

#include "CAPITAL_Engine.h"
#include "Control.h"
#include "Mechanics.h"
#include "Pipelines.h"
#include "Window.h"
#include "World.h"

VulkanMechanics::VulkanMechanics() {
  _log.console("{ #?# }", "constructing Vulkan Mechanics");
}

VulkanMechanics::~VulkanMechanics() {
  _log.console("{ #?# }", "destructing Vulkan Mechanics");
}

void VulkanMechanics::createInstance() {
  _log.console("{ VKI }", "creating Vulkan Instance");
  if (_validationLayers.enableValidationLayers &&
      !_validationLayers.checkValidationLayerSupport()) {
    throw std::runtime_error("validation layers requested, but not available!");
  }

  VkApplicationInfo appInfo{};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "CAPITAL";
  appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
  appInfo.pEngineName = "CAPITAL engine";
  appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
  appInfo.apiVersion = VK_API_VERSION_1_3;
  _log.console(
      _log.style.dashLeader, "Application name:", appInfo.pApplicationName,
      "\n", _log.style.indentSize, _log.style.dashLeader,
      "Application Version:", appInfo.applicationVersion, "\n",
      _log.style.indentSize, _log.style.dashLeader,
      "Engine Name Version:", appInfo.pEngineName, "\n", _log.style.indentSize,
      _log.style.dashLeader, "Engine Version:", appInfo.engineVersion, "\n",
      _log.style.indentSize, _log.style.dashLeader, "API Version:", 1.3);

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  auto extensions = getRequiredExtensions();
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
  if (_validationLayers.enableValidationLayers) {
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(_validationLayers.validationLayers.size());
    createInfo.ppEnabledLayerNames = _validationLayers.validationLayers.data();

    _validationLayers.populateDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
  } else {
    createInfo.enabledLayerCount = 0;

    createInfo.pNext = nullptr;
  }

  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
    throw std::runtime_error("failed to create instance!");
  }
}

void VulkanMechanics::createSurface() {
  _log.console("{ [ ] }", "creating Surface");
  if (glfwCreateWindowSurface(instance, _window.window, nullptr, &surface) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
  }
}

void VulkanMechanics::pickPhysicalDevice() {
  _log.console("{ ### }", "picking Physical Device");
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  for (const auto& device : devices) {
    if (isDeviceSuitable(device)) {
      mainDevice.physical = device;
      break;
    }
  }

  if (mainDevice.physical == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }
}

VulkanMechanics::Queues::FamilyIndices VulkanMechanics::findQueueFamilies(
    VkPhysicalDevice physicalDevice) {
  _log.console(_log.style.dashLeader, "finding Queue Families");

  VulkanMechanics::Queues::FamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount,
                                           queueFamilies.data());

  int i = 0;
  for (const auto& queueFamily : queueFamilies) {
    if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
        (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
      indices.graphicsAndComputeFamily = i;
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

VulkanMechanics::SwapChain::SupportDetails
VulkanMechanics::querySwapChainSupport(VkPhysicalDevice physicalDevice) {
  _log.console(_log.style.dashLeader, "querying Swap Chain Support");
  {
    SwapChain::SupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface,
                                              &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount,
                                         nullptr);

    if (formatCount != 0) {
      details.formats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(
          physicalDevice, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
                                              &presentModeCount, nullptr);

    if (presentModeCount != 0) {
      details.presentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
                                                &presentModeCount,
                                                details.presentModes.data());
    }

    return details;
  }
}

bool VulkanMechanics::checkDeviceExtensionSupport(
    VkPhysicalDevice physicalDevice) {
  _log.console(_log.style.dashLeader, "checking Device Extension Support");
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

void VulkanMechanics::createLogicalDevice() {
  _log.console("{ +++ }", "creating Logical Device");
  Queues::FamilyIndices indices = findQueueFamilies(mainDevice.physical);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {
      indices.graphicsAndComputeFamily.value(), indices.presentFamily.value()};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures{};

  VkDeviceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  createInfo.queueCreateInfoCount =
      static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos = queueCreateInfos.data();

  createInfo.pEnabledFeatures = &deviceFeatures;

  createInfo.enabledExtensionCount =
      static_cast<uint32_t>(deviceExtensions.size());
  createInfo.ppEnabledExtensionNames = deviceExtensions.data();

  _log.console(_log.style.dashLeader,
               "Enabled Extension Names:", *createInfo.ppEnabledExtensionNames);

  if (_validationLayers.enableValidationLayers) {
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(_validationLayers.validationLayers.size());
    createInfo.ppEnabledLayerNames = _validationLayers.validationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }

  if (vkCreateDevice(mainDevice.physical, &createInfo, nullptr,
                     &mainDevice.logical) != VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
  }

  vkGetDeviceQueue(mainDevice.logical, indices.graphicsAndComputeFamily.value(),
                   0, &queues.graphics);
  vkGetDeviceQueue(mainDevice.logical, indices.graphicsAndComputeFamily.value(),
                   0, &queues.compute);
  vkGetDeviceQueue(mainDevice.logical, indices.presentFamily.value(), 0,
                   &queues.present);
}

VkSurfaceFormatKHR VulkanMechanics::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats) {
  _log.console(_log.style.dashLeader, "choosing Swap Surface Format");

  for (const auto& availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

VkPresentModeKHR VulkanMechanics::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& availablePresentModes) {
  _log.console(_log.style.dashLeader, "choosing Swap Present Mode");
  for (const auto& availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return availablePresentMode;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanMechanics::chooseSwapExtent(
    const VkSurfaceCapabilitiesKHR& capabilities) {
  _log.console(_log.style.dashLeader, "choosing Swap Extent");

  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(_window.window, &width, &height);

    VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                               static_cast<uint32_t>(height)};

    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);

    return actualExtent;
  }
}

void VulkanMechanics::createSyncObjects() {
  _log.console("{ ||| }", "creating Sync Objects");

  syncObjects.imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  syncObjects.renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  syncObjects.computeFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  syncObjects.inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
  syncObjects.computeInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(_mechanics.mainDevice.logical, &semaphoreInfo,
                          nullptr, &syncObjects.imageAvailableSemaphores[i]) !=
            VK_SUCCESS ||
        vkCreateSemaphore(_mechanics.mainDevice.logical, &semaphoreInfo,
                          nullptr, &syncObjects.renderFinishedSemaphores[i]) !=
            VK_SUCCESS ||
        vkCreateFence(_mechanics.mainDevice.logical, &fenceInfo, nullptr,
                      &syncObjects.inFlightFences[i]) != VK_SUCCESS) {
      throw std::runtime_error(
          "failed to create graphics synchronization objects for a frame!");
    }
    if (vkCreateSemaphore(_mechanics.mainDevice.logical, &semaphoreInfo,
                          nullptr, &syncObjects.computeFinishedSemaphores[i]) !=
            VK_SUCCESS ||
        vkCreateFence(_mechanics.mainDevice.logical, &fenceInfo, nullptr,
                      &syncObjects.computeInFlightFences[i]) != VK_SUCCESS) {
      throw std::runtime_error(
          "failed to create compute synchronization objects for a frame!");
    }
  }
}

void VulkanMechanics::cleanupSwapChain() {
  for (auto framebuffer : swapChain.framebuffers) {
    vkDestroyFramebuffer(_mechanics.mainDevice.logical, framebuffer, nullptr);
  }

  for (auto imageView : swapChain.imageViews) {
    vkDestroyImageView(_mechanics.mainDevice.logical, imageView, nullptr);
  }

  vkDestroySwapchainKHR(_mechanics.mainDevice.logical, swapChain.swapChain,
                        nullptr);
}

bool VulkanMechanics::isDeviceSuitable(VkPhysicalDevice physicalDevice) {
  _log.console(_log.style.dashLeader,
               "checking if Physical Device is suitable");

  Queues::FamilyIndices indices = findQueueFamilies(physicalDevice);

  bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);

  bool swapChainAdequate = false;
  if (extensionsSupported) {
    SwapChain::SupportDetails swapChainSupport =
        querySwapChainSupport(physicalDevice);
    swapChainAdequate = !swapChainSupport.formats.empty() &&
                        !swapChainSupport.presentModes.empty();
  }

  return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

void VulkanMechanics::createSwapChain() {
  _log.console("{ <-> }", "creating Swap Chain");
  SwapChain::SupportDetails swapChainSupport =
      querySwapChainSupport(mainDevice.physical);

  VkSurfaceFormatKHR surfaceFormat =
      chooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode =
      chooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapChainSupport.capabilities.maxImageCount) {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = surface;

  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  Queues::FamilyIndices indices = findQueueFamilies(mainDevice.physical);
  uint32_t queueFamilyIndices[] = {indices.graphicsAndComputeFamily.value(),
                                   indices.presentFamily.value()};

  if (indices.graphicsAndComputeFamily != indices.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  if (vkCreateSwapchainKHR(mainDevice.logical, &createInfo, nullptr,
                           &swapChain.swapChain) != VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain!");
  }

  vkGetSwapchainImagesKHR(mainDevice.logical, swapChain.swapChain, &imageCount,
                          nullptr);
  swapChain.images.resize(imageCount);
  vkGetSwapchainImagesKHR(mainDevice.logical, swapChain.swapChain, &imageCount,
                          swapChain.images.data());

  swapChain.imageFormat = surfaceFormat.format;
  swapChain.extent = extent;
}

void VulkanMechanics::recreateSwapChain() {
  int width = 0, height = 0;
  glfwGetFramebufferSize(_window.window, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(_window.window, &width, &height);
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(mainDevice.logical);

  cleanupSwapChain();

  createSwapChain();
  createImageViews();
  _memCommands.createFramebuffers();
}

std::vector<const char*> VulkanMechanics::getRequiredExtensions() {
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char*> extensions(glfwExtensions,
                                      glfwExtensions + glfwExtensionCount);

  if (_validationLayers.enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}

void VulkanMechanics::createImageViews() {
  _log.console(_log.style.dashLeader, "creating Image Views");
  swapChain.imageViews.resize(swapChain.images.size());

  for (size_t i = 0; i < swapChain.images.size(); i++) {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = swapChain.images[i];
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = swapChain.imageFormat;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(mainDevice.logical, &createInfo, nullptr,
                          &swapChain.imageViews[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create image views!");
    }
  }
}
