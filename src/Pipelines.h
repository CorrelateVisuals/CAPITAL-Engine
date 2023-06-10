#pragma once

#include <glm/glm.hpp>

class Pipelines {
 public:
  Pipelines();
  ~Pipelines();

  struct Graphics {
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    std::vector<VkShaderModule> shaderModules;
  } graphics;

  struct Compute {
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    std::vector<VkShaderModule> shaderModules;
  } compute;

  VkRenderPass renderPass;

  struct Depth {
    VkImage image;
    VkDeviceMemory imageMemory;
    VkImageView imageView;
  } depth;

  struct MultiSampling {
    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;
  } msaa;

 public:
  void createColorResources();
  void createDepthResources();

  void createRenderPass();

  void createGraphicsPipeline();
  void createComputePipeline();

  VkSampleCountFlagBits getMaxUsableSampleCount();

 private:
  VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
                               VkImageTiling tiling,
                               VkFormatFeatureFlags features);
  VkFormat findDepthFormat();
  bool hasStencilComponent(VkFormat format);

  static std::vector<char> readShaderFile(const std::string& filename);
  VkShaderModule createShaderModule(const std::vector<char>& code);
  VkPipelineShaderStageCreateInfo getShaderStageInfo(
      VkShaderStageFlagBits shaderStage,
      std::string shaderName,
      auto pipeline);

  VkPipelineVertexInputStateCreateInfo getVertexInputInfo();
  VkPipelineColorBlendStateCreateInfo getColorBlend();
  VkPipelineDynamicStateCreateInfo getDynamicState();
};

class MemoryCommands {
 public:
  MemoryCommands();
  ~MemoryCommands();

  struct PushConstants {
    VkShaderStageFlagBits shaderStage = {VK_SHADER_STAGE_COMPUTE_BIT};
    uint32_t offset = 0;
    uint32_t size = 128;
    std::array<int, 32> data = {};
  } pushConstants;

  struct UniformBuffers {
    std::vector<VkBuffer> buffers;
    std::vector<VkDeviceMemory> buffersMemory;
    std::vector<void*> buffersMapped;
  } uniform;

  struct ShaderStorageBuffers {
    std::vector<VkBuffer> buffers;
    std::vector<VkDeviceMemory> buffersMemory;
  } shaderStorage;

  struct Descriptor {
    VkDescriptorPool pool;
    std::vector<VkDescriptorSet> sets;
    VkDescriptorSetLayout setLayout;
  } descriptor;

  struct Command {
    VkCommandPool pool;
    std::vector<VkCommandBuffer> graphicBuffers;
    std::vector<VkCommandBuffer> computeBuffers;
  } command;

 public:
  void createFramebuffers();

  void createCommandPool();
  void createCommandBuffers();

  void createShaderStorageBuffers();

  void createUniformBuffers();
  void updateUniformBuffer(uint32_t currentImage);

  void createDescriptorSetLayout();
  void createDescriptorPool();

  void createDescriptorSets();
  void createComputeCommandBuffers();

  void recordComputeCommandBuffer(VkCommandBuffer commandBuffer);
  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

  void createImage(uint32_t width,
                   uint32_t height,
                   VkSampleCountFlagBits numSamples,
                   VkFormat format,
                   VkImageTiling tiling,
                   VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties,
                   VkImage& image,
                   VkDeviceMemory& imageMemory);

 private:
  void createBuffer(VkDeviceSize size,
                    VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties,
                    VkBuffer& buffer,
                    VkDeviceMemory& bufferMemory);
  uint32_t findMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties);
  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};
