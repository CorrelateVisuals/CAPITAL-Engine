#pragma once

#include "vulkan/vulkan.h"

#include "array"
#include "vector"

class MemoryCommands {
 public:
  MemoryCommands();
  ~MemoryCommands();

  struct PushConstants {
    VkShaderStageFlags shaderStage = {VK_SHADER_STAGE_COMPUTE_BIT};
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