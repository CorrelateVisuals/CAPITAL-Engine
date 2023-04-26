#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"

#include <algorithm>
#include <set>
#include <stdexcept>

#include "Capital_Engine.h"
#include "Debug.h"
#include "Pipelines.h"
#include "Settings.h"
#include "Vulkan_Mechanics.h"
#include "Window.h"
#include "World.h"

VulkanMechanics::VulkanMechanics()
    : surface{},
      instance{},
      mainDevice{VK_NULL_HANDLE, VK_NULL_HANDLE},
      deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME},
      queues{},
      queueFamilyIndices{},
      swapChainSupport{},
      swapChain{},
      swapChainImages{},
      swapChainImageFormat{},
      swapChainExtent{},
      imageAvailableSemaphores{},
      renderFinishedSemaphores{},
      inFlightFences{} {
  LOG.console("{ # }", "constructing Vulkan Mechanics");
}

VulkanMechanics::~VulkanMechanics() {
  LOG.console("{ # }", "destructing Vulkan Mechanics");
}

void VulkanMechanics::createInstance() {
  LOG.console("{ vk }", "creating Vulkan Instance");

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
  if (LOG_LAYERS.enableValidationLayers) {
    if (!LOG_LAYERS.checkValidationLayerSupport()) {
      throw std::runtime_error(
          "validation layers requested, but not available!");
    }
    instanceCreateInfo.enabledLayerCount =
        static_cast<uint32_t>(LOG_LAYERS.validationLayers.size());
    instanceCreateInfo.ppEnabledLayerNames = LOG_LAYERS.validationLayers.data();

    LOG_LAYERS.populateDebugMessengerCreateInfo(debugCreateInfo);
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
  LOG.console("{ [] }", "creating Surface");

  if (glfwCreateWindowSurface(instance, WINDOW.window, nullptr, &surface) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
  }
}

void VulkanMechanics::pickPhysicalDevice() {
  LOG.console("{ ## }", "picking Physical Device");

  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

  for (VkPhysicalDevice device : devices) {
    if (isDeviceSuitable(device)) {
      mainDevice.physical = device;

      VkPhysicalDeviceProperties deviceProperties;
      vkGetPhysicalDeviceProperties(device, &deviceProperties);
      LOG.console("{ ## }", "GPU picked:", deviceProperties.deviceName);
      break;
    }
  }

  if (mainDevice.physical == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }
}

VulkanMechanics::QueueFamilyIndices VulkanMechanics::findQueueFamilies(
    VkPhysicalDevice physical) {
  LOG.console("  ....  ", "finding Queue Families");

  VulkanMechanics::QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physical, &queueFamilyCount,
                                           nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physical, &queueFamilyCount,
                                           queueFamilies.data());
  int i = 0;
  for (const auto& queueFamily : queueFamilies) {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
    }
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(physical, i, surface, &presentSupport);
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

bool VulkanMechanics::checkDeviceExtensionSupport(VkPhysicalDevice physical) {
  LOG.console("  ....  ", "checking Device Extension Support");

  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(physical, nullptr, &extensionCount,
                                       nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(physical, nullptr, &extensionCount,
                                       availableExtensions.data());
  std::set<std::string> requiredExtensions(deviceExtensions.begin(),
                                           deviceExtensions.end());

  for (const auto& extension : availableExtensions) {
    requiredExtensions.erase(extension.extensionName);
  }
  return requiredExtensions.empty();
}

void VulkanMechanics::createLogicalDevice() {
  LOG.console("{ ++ }", "creating Logical Device");

  VulkanMechanics::QueueFamilyIndices indices =
      findQueueFamilies(mainDevice.physical);

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(),
                                            indices.presentFamily.value()};

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

  if (LOG_LAYERS.enableValidationLayers) {
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(LOG_LAYERS.validationLayers.size());
    createInfo.ppEnabledLayerNames = LOG_LAYERS.validationLayers.data();
  } else {
    createInfo.enabledLayerCount = 0;
  }

  if (vkCreateDevice(mainDevice.physical, &createInfo, nullptr,
                     &mainDevice.logical) != VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
  }

  vkGetDeviceQueue(mainDevice.logical, indices.graphicsFamily.value(), 0,
                   &queues.graphics);
  vkGetDeviceQueue(mainDevice.logical, indices.graphicsFamily.value(), 0,
                   &queues.compute);
  vkGetDeviceQueue(mainDevice.logical, indices.presentFamily.value(), 0,
                   &queues.present);
}

VkSurfaceFormatKHR VulkanMechanics::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats) {
  LOG.console("  ....  ", "choosing Swap Surface Format");
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
  LOG.console("  ....  ", "choosing Swap Present Mode");
  for (const auto& availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return availablePresentMode;
    }
  }
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanMechanics::chooseSwapExtent(
    const VkSurfaceCapabilitiesKHR& capabilities) {
  LOG.console("  ....  ", "choosing Swap Extent");
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(WINDOW.window, &width, &height);

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
  LOG.console("{ || }", "creating Sync Objects");

  imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  computeFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
  computeInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(MECHANICS.mainDevice.logical, &semaphoreInfo, nullptr,
                          &imageAvailableSemaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(MECHANICS.mainDevice.logical, &semaphoreInfo, nullptr,
                          &renderFinishedSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(MECHANICS.mainDevice.logical, &fenceInfo, nullptr,
                      &inFlightFences[i]) != VK_SUCCESS) {
      throw std::runtime_error(
          "failed to create graphics synchronization objects for a frame!");
    }
    if (vkCreateSemaphore(MECHANICS.mainDevice.logical, &semaphoreInfo, nullptr,
                          &computeFinishedSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(MECHANICS.mainDevice.logical, &fenceInfo, nullptr,
                      &computeInFlightFences[i]) != VK_SUCCESS) {
      throw std::runtime_error(
          "failed to create compute synchronization objects for a frame!");
    }
  }
}

void VulkanMechanics::cleanupSwapChain() {
  for (auto framebuffer : swapChainFramebuffers) {
    vkDestroyFramebuffer(MECHANICS.mainDevice.logical, framebuffer, nullptr);
  }

  for (auto imageView : swapChainImageViews) {
    vkDestroyImageView(MECHANICS.mainDevice.logical, imageView, nullptr);
  }

  vkDestroySwapchainKHR(MECHANICS.mainDevice.logical, swapChain, nullptr);
}

bool VulkanMechanics::isDeviceSuitable(VkPhysicalDevice physical) {
  LOG.console("  ....  ", "checking if Physical Device is suitable");

  QueueFamilyIndices indices = findQueueFamilies(physical);

  bool extensionsSupported = checkDeviceExtensionSupport(physical);

  bool swapChainAdequate = false;
  if (extensionsSupported) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physical);
    swapChainAdequate = !swapChainSupport.formats.empty() &&
                        !swapChainSupport.presentModes.empty();
  }

  return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

void VulkanMechanics::createSwapChain() {
  LOG.console("{ <-> }", "creating Swap Chain");
  VulkanMechanics::SwapChainSupportDetails swapChainSupport =
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

  VulkanMechanics::QueueFamilyIndices indices =
      findQueueFamilies(mainDevice.physical);
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                   indices.presentFamily.value()};

  if (indices.graphicsFamily != indices.presentFamily) {
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

  createInfo.oldSwapchain = VK_NULL_HANDLE;

  if (vkCreateSwapchainKHR(mainDevice.logical, &createInfo, nullptr,
                           &swapChain) != VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain!");
  }

  vkGetSwapchainImagesKHR(mainDevice.logical, swapChain, &imageCount, nullptr);
  swapChainImages.resize(imageCount);
  vkGetSwapchainImagesKHR(mainDevice.logical, swapChain, &imageCount,
                          swapChainImages.data());

  swapChainImageFormat = surfaceFormat.format;
  swapChainExtent = extent;
}

void VulkanMechanics::recreateSwapChain() {
  int width = 0, height = 0;
  glfwGetFramebufferSize(WINDOW.window, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(WINDOW.window, &width, &height);
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(MECHANICS.mainDevice.logical);

  cleanupSwapChain();

  createSwapChain();
  RENDER_CONFIG.createImageViews();
  RENDER_CONFIG.createDepthResources();
  RENDER_CONFIG.createFrameBuffers();
}

std::vector<const char*> VulkanMechanics::getRequiredExtensions() {
  LOG.console("  ....  ", "acquiring Required Extensions");

  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char*> extensions(glfwExtensions,
                                      glfwExtensions + glfwExtensionCount);

  if (LOG_LAYERS.enableValidationLayers) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}

VulkanMechanics::SwapChainSupportDetails VulkanMechanics::querySwapChainSupport(
    VkPhysicalDevice physical) {
  LOG.console("  ....  ", "querying Swap Chain Support");

  VulkanMechanics::SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical, surface,
                                            &details.capabilities);

  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical, surface, &formatCount,
                                       nullptr);
  details.formats.resize(formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(physical, surface, &formatCount,
                                       details.formats.data());

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(physical, surface,
                                            &presentModeCount, nullptr);
  details.presentModes.resize(presentModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(
      physical, surface, &presentModeCount, details.presentModes.data());

  return details;
}

RenderConfiguration::RenderConfiguration()
    : depthImage{},
      depthImageMemory{},
      depthImageView{},
      renderPass{VK_NULL_HANDLE} {
  LOG.console("{ < }", "constructing Render Configuration");
}

RenderConfiguration::~RenderConfiguration() {
  LOG.console("{ < }", "constructing Render Configuration");
}

void RenderConfiguration::createDepthResources() {
  LOG.console("{ -z }", "creating Depth Resources");

  VkFormat depthFormat = findDepthFormat();
  createImage(MECHANICS.swapChainExtent.width, MECHANICS.swapChainExtent.height,
              depthFormat, VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage,
              depthImageMemory);
  depthImageView =
      createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void RenderConfiguration::createImageViews() {
  LOG.console("  ....  ", "creating Image Views");
  MECHANICS.swapChainImageViews.resize(MECHANICS.swapChainImages.size());

  for (size_t i = 0; i < MECHANICS.swapChainImages.size(); i++) {
    MECHANICS.swapChainImageViews[i] = createImageView(
        MECHANICS.swapChainImages[i], MECHANICS.swapChainImageFormat,
        VK_IMAGE_ASPECT_COLOR_BIT);

    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = MECHANICS.swapChainImages[i];
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = MECHANICS.swapChainImageFormat;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(MECHANICS.mainDevice.logical, &createInfo, nullptr,
                          &MECHANICS.swapChainImageViews[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create image views!");
    }
  }
}

VkFormat RenderConfiguration::findDepthFormat() {
  LOG.console("  ....  ", "finding Depth Format");
  ;
  hasStencilComponent(depthFormat);

  return findSupportedFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
       VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkFormat RenderConfiguration::findSupportedFormat(
    const std::vector<VkFormat>& candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features) {
  LOG.console("  ....  ", "finding Supported Format");

  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(MECHANICS.mainDevice.physical, format,
                                        &props);

    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
               (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  throw std::runtime_error("failed to find supported format!");
}

bool RenderConfiguration::hasStencilComponent(VkFormat format) {
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
         format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void RenderConfiguration::createRenderPass() {
  LOG.console("{ RP }", "creating Render Pass");
  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = MECHANICS.swapChainImageFormat;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentDescription depthAttachment{};
  depthAttachment.format = findDepthFormat();
  depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthAttachmentRef{};
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;
  subpass.pDepthStencilAttachment = &depthAttachmentRef;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  std::array<VkAttachmentDescription, 2> attachments = {colorAttachment,
                                                        depthAttachment};
  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  if (vkCreateRenderPass(MECHANICS.mainDevice.logical, &renderPassInfo, nullptr,
                         &renderPass) != VK_SUCCESS) {
    throw std::runtime_error("failed to create render pass!");
  }
}

void RenderConfiguration::createImage(uint32_t width,
                                      uint32_t height,
                                      VkFormat format,
                                      VkImageTiling tiling,
                                      VkImageUsageFlags usage,
                                      VkMemoryPropertyFlags properties,
                                      VkImage& image,
                                      VkDeviceMemory& imageMemory) {
  LOG.console("  ....  ", "creating Image");

  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateImage(MECHANICS.mainDevice.logical, &imageInfo, nullptr,
                    &image) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image!");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(MECHANICS.mainDevice.logical, image,
                               &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(MECHANICS.mainDevice.logical, &allocInfo, nullptr,
                       &imageMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate image memory!");
  }
  vkBindImageMemory(MECHANICS.mainDevice.logical, image, imageMemory, 0);
}

void RenderConfiguration::createFrameBuffers() {
  LOG.console("{ [0] }", "creating Frame Buffers");

  MECHANICS.swapChainFramebuffers.resize(MECHANICS.swapChainImageViews.size());

  VkImageView attachments[2] = {0};

  for (size_t i = 0; i < MECHANICS.swapChainImageViews.size(); i++) {
    attachments[0] = MECHANICS.swapChainImageViews[i];
    attachments[1] = depthImageView;

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 2;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = MECHANICS.swapChainExtent.width;
    framebufferInfo.height = MECHANICS.swapChainExtent.height;
    framebufferInfo.layers = 1;

    LOG.console("  ....  ", "creating framebuffer", i, "of",
                MECHANICS.swapChainImageViews.size());

    if (vkCreateFramebuffer(MECHANICS.mainDevice.logical, &framebufferInfo,
                            nullptr, &MECHANICS.swapChainFramebuffers[i]) !=
        VK_SUCCESS) {
      throw std::runtime_error("Failed to create framebuffer!");
    }
  }
}

VkImageView RenderConfiguration::createImageView(
    VkImage image,
    VkFormat format,
    VkImageAspectFlags aspectFlags) {
  LOG.console("  ....  ", "creating Image View");

  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = aspectFlags;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  VkImageView imageView;
  if (vkCreateImageView(MECHANICS.mainDevice.logical, &viewInfo, nullptr,
                        &imageView) != VK_SUCCESS) {
    throw std::runtime_error("failed to create texture image view!");
  }

  return imageView;
}

uint32_t RenderConfiguration::findMemoryType(uint32_t typeFilter,
                                             VkMemoryPropertyFlags properties) {
  LOG.console("  ....  ", "finding Memory Type");

  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(MECHANICS.mainDevice.physical,
                                      &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags &
                                    properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}
