#pragma once

class Pipelines {
 public:
  VkDescriptorSetLayout descriptorSetLayout;

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
};
inline Pipelines pipelines;

class MemoryCommands {
 public:
  MemoryCommands();
  ~MemoryCommands();

  struct UniformBufferObject {
    float deltaTime = 1.0f;
  };

  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers;

  void createCommandPool();
  void createCommandBuffers();

  void createShaderStorageBuffers();
  void createUniformBuffers();

 private:
  std::vector<VkBuffer> shaderStorageBuffers;
  std::vector<VkDeviceMemory> shaderStorageBuffersMemory;

  std::vector<VkBuffer> uniformBuffers;
  std::vector<VkDeviceMemory> uniformBuffersMemory;
  std::vector<void*> uniformBuffersMapped;

  void createBuffer(VkDeviceSize size,
                    VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties,
                    VkBuffer& buffer,
                    VkDeviceMemory& bufferMemory);
  void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
  uint32_t findMemoryType(uint32_t typeFilter,
                          VkMemoryPropertyFlags properties);
};
inline MemoryCommands memCommands;
