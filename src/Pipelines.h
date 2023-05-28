#pragma once

#include <glm/glm.hpp>

class Pipelines {
 public:
  Pipelines() = default;
  ~Pipelines() = default;

  struct Graphics {
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
  } graphics;

  struct Compute {
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
  } compute;

  VkRenderPass renderPass;

  struct Depth {
    VkImage image;
    VkDeviceMemory imageMemory;
    VkImageView imageView;
  } depth;

 public:
  void createDepthResources();
  void createRenderPass();

  void createGraphicsPipeline();
  void createComputePipeline();

 private:
  VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
                               VkImageTiling tiling,
                               VkFormatFeatureFlags features);
  VkFormat findDepthFormat();
  bool hasStencilComponent(VkFormat format);

  static std::vector<char> readShaderFile(const std::string& filename);
  VkShaderModule createShaderModule(const std::vector<char>& code);
};

class MemoryCommands {
 public:
  MemoryCommands();
  ~MemoryCommands();

  struct Command {
    VkCommandPool pool;
    std::vector<VkCommandBuffer> graphicBuffers;
    std::vector<VkCommandBuffer> computeBuffers;
  } command;

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

 public:
  void createFramebuffers();

  void createCommandPool();
  void createCommandBuffers();

  void createShaderStorageBuffers();

  void createUniformBuffers();
  void updateUniformBuffer(uint32_t currentImage);

  void createComputeDescriptorSetLayout();
  void createDescriptorPool();

  void createComputeDescriptorSets();
  void createComputeCommandBuffers();

  void recordComputeCommandBuffer(VkCommandBuffer commandBuffer);
  void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

  void createImage(uint32_t width,
                   uint32_t height,
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
