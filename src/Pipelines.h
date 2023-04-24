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

  void createShaderStorageBuffers();

 private:
  std::vector<VkBuffer> shaderStorageBuffers;
  std::vector<VkDeviceMemory> shaderStorageBuffersMemory;

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
