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
