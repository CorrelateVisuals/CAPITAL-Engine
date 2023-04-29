#pragma once
#include "Debug.h"
#include "Mechanics.h"
#include "Pipelines.h"
#include "Window.h"
#include "World.h"

class CapitalEngine {
 public:
  CapitalEngine();
  ~CapitalEngine();

  void mainLoop();

 private:
  void initVulkan();
  void drawFrame();
};

class Globals {
 public:
  Globals() = default;
  ~Globals();

  void cleanup();

  class Objects {
   public:
    Objects() = default;
    ~Objects() = default;

    Logging logging;
    ValidationLayers validationLayers;
    Window mainWindow;
    VulkanMechanics vulkanMechanics;
    RenderConfiguration renderConfig;
    Pipelines pipelines;
    MemoryCommands memoryCommands;
    World world;
  };
  inline static Objects obj;
};

constexpr auto& _log = Globals::obj.logging;
constexpr auto& _validationLayers = Globals::obj.validationLayers;
constexpr auto& _window = Globals::obj.mainWindow;
constexpr auto& _mechanics = Globals::obj.vulkanMechanics;
constexpr auto& _renderConfig = Globals::obj.renderConfig;
constexpr auto& _memCommands = Globals::obj.memoryCommands;
constexpr auto& _pipelines = Globals::obj.pipelines;
constexpr auto& _world = Globals::obj.world;

#pragma once

#include <array>
#include <string>

class Control {
 public:
  double timer();
};

struct DisplayConfig {
 public:
  const char* windowTitle = "CAPITAL engine";
  uint32_t width = 1920;
  uint32_t height = 1080;
};
inline DisplayConfig displayConfig;

#pragma once

class Pipelines {
 public:
  Pipelines();
  ~Pipelines();

  struct Graphics {
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
  } graphics;

  struct Compute {
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
  } compute;

  void createDescriptorSetLayout();
  void createGraphicsPipeline();
  void createComputePipeline();

 private:
  std::vector<char> readShaderFiles(const std::string& filename);
  VkShaderModule createShaderModule(const std::vector<char>& code);

 public:
  VkImage depthImage;
  VkDeviceMemory depthImageMemory;
  VkImageView depthImageView;
  VkFormat depthFormat;

  VkRenderPass renderPass;

  void createImageViews();
  void createRenderPass();

  void createDepthResources();
  void createFrameBuffers();

 private:
  VkFormat findDepthFormat();
  VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
                               VkImageTiling tiling,
                               VkFormatFeatureFlags features);
  bool hasStencilComponent(VkFormat format);
  void createImage(uint32_t width,
                   uint32_t height,
                   VkFormat format,
                   VkImageTiling tiling,
                   VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties,
                   VkImage& image,
                   VkDeviceMemory& imageMemory);

  VkImageView createImageView(VkImage image,
                              VkFormat format,
                              VkImageAspectFlags aspectFlags);
  uint32_t findMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties);
};

class MemoryCommands {
 public:
  MemoryCommands();
  ~MemoryCommands();

  struct UniformBufferObject {
    float deltaTime = 1.0f;
  };

  float lastFrameTime = 0.0f;
  double lastTime = 0.0f;

  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers;
  std::vector<VkCommandBuffer> computeCommandBuffers;

  VkDescriptorSetLayout descriptorSetLayout;
  VkDescriptorPool descriptorPool;
  std::vector<VkDescriptorSet> computeDescriptorSets;

  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkDeviceMemory> uniformBuffersMemory;
  std::vector<void*> uniformBuffersMapped;

  std::vector<VkBuffer> shaderStorageBuffers;
  std::vector<VkDeviceMemory> shaderStorageBuffersMemory;

  void createCommandPool();
  void createCommandBuffers();
  void createComputeCommandBuffers();

  void createShaderStorageBuffers();
  void createUniformBuffers();

  void createDescriptorPool();
  void createComputeDescriptorSets();

  void updateUniformBuffer(uint32_t currentImage);

  void recordComputeCommandBuffer(VkCommandBuffer commandBuffer);
  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

 private:
  void createBuffer(VkDeviceSize size,
                    VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties,
                    VkBuffer& buffer,
                    VkDeviceMemory& bufferMemory);
  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
  uint32_t findMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties);
};

#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "CAPITAL_Engine.h"

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

class VulkanMechanics {
 public:
  VulkanMechanics();
  ~VulkanMechanics();

  VkSurfaceKHR surface;
  VkInstance instance;

  struct Device {
    VkPhysicalDevice physical;
    VkDevice logical;
  } mainDevice;

  struct Queues {
    VkQueue graphics;
    VkQueue compute;
    VkQueue present;
  } queues;

  struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    bool isComplete() const {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  } queueFamilyIndices;

  VkSwapchainKHR swapChain;
  std::vector<VkImage> swapChainImages;
  VkFormat swapChainImageFormat;
  std::vector<VkImageView> swapChainImageViews;
  VkExtent2D swapChainExtent;
  std::vector<VkFramebuffer> swapChainFramebuffers;

  std::vector<VkSemaphore> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
  std::vector<VkSemaphore> computeFinishedSemaphores;
  std::vector<VkFence> inFlightFences;
  std::vector<VkFence> computeInFlightFences;
  uint32_t currentFrame = 0;

  void createInstance();
  void createSurface();

  void pickPhysicalDevice();
  void createLogicalDevice();

  void createSwapChain();
  void recreateSwapChain();

  void createSyncObjects();

  void cleanupSwapChain();

  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physical);

 private:
  const std::vector<const char*> deviceExtensions;

  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  } swapChainSupport;

  std::vector<const char*> getRequiredExtensions();
  bool isDeviceSuitable(VkPhysicalDevice physical);
  bool checkDeviceExtensionSupport(VkPhysicalDevice physical);
  SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physical);
  VkSurfaceFormatKHR chooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR>& availableFormats);
  VkPresentModeKHR chooseSwapPresentMode(
      const std::vector<VkPresentModeKHR>& availablePresentModes);
  VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
};

class RenderConfiguration {
 public:
  RenderConfiguration();
  ~RenderConfiguration();

  VkImage depthImage;
  VkDeviceMemory depthImageMemory;
  VkImageView depthImageView;
  VkFormat depthFormat;

  VkRenderPass renderPass;

  void createImageViews();
  void createRenderPass();

  void createDepthResources();
  void createFrameBuffers();

 private:
  VkFormat findDepthFormat();
  VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
                               VkImageTiling tiling,
                               VkFormatFeatureFlags features);
  bool hasStencilComponent(VkFormat format);
  void createImage(uint32_t width,
                   uint32_t height,
                   VkFormat format,
                   VkImageTiling tiling,
                   VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties,
                   VkImage& image,
                   VkDeviceMemory& imageMemory);

  VkImageView createImageView(VkImage image,
                              VkFormat format,
                              VkImageAspectFlags aspectFlags);
  uint32_t findMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties);
};

#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include <array>
#include <vector>

#include "Control.h"

constexpr int CELL_COUNT = 4;

class World {
 public:
  World();
  ~World();

  struct Cell {
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec4 color;

    static VkVertexInputBindingDescription getBindingDescription() {
      VkVertexInputBindingDescription bindingDescription{};
      bindingDescription.binding = 0;
      bindingDescription.stride = sizeof(Cell);
      bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2>
    getAttributeDescriptions() {
      std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

      attributeDescriptions[0].binding = 0;
      attributeDescriptions[0].location = 0;
      attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
      attributeDescriptions[0].offset = offsetof(Cell, position);

      attributeDescriptions[1].binding = 0;
      attributeDescriptions[1].location = 1;
      attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
      attributeDescriptions[1].offset = offsetof(Cell, color);

      return attributeDescriptions;
    }
  };
};
