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
