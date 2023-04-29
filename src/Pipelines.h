#pragma once

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

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

  struct Descriptor {
    VkDescriptorPool pool;
    std::vector<VkDescriptorSet> sets;
    VkDescriptorSetLayout setLayout;
  } descriptor;

 public:
  void createRenderPass();
  void createComputeDescriptorSetLayout();

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

  float lastFrameTime = 0.0f;
  double lastTime = 0.0f;

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

 public:
  void createFramebuffers();
  void createCommandPool();
};
