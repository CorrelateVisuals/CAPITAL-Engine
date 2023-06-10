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
  void destroyShaderModules(std::vector<VkShaderModule>& shaderModules);

  VkPipelineVertexInputStateCreateInfo getVertexInputInfo();
  VkPipelineColorBlendStateCreateInfo getColorBlendingInfo();
  VkPipelineDynamicStateCreateInfo getDynamicStateInfo();
  VkPipelineDepthStencilStateCreateInfo getDepthStencilInfo();
};
