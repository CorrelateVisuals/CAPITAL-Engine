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

  VkRenderPass renderPass;

 public:
  void createRenderPass();

  void createGraphicsPipeline();
  void createComputePipeline();

 private:
  static std::vector<char> readShaderFile(const std::string& filename);
  VkShaderModule createShaderModule(const std::vector<char>& code);
};

class MemoryCommands {
 public:
  MemoryCommands();
  ~MemoryCommands();

  struct UniformBufferObject {
    float deltaTime = 1.0f;
  };

  // float lastFrameTime = 0.0f;
  // double lastTime = 0.0f;

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
