#pragma once

class Pipelines {
 public:
  VkDescriptorSetLayout descriptorSetLayout;
  VkPipelineLayout pipelineLayout;
  VkPipeline graphicsPipeline;

  void createDescriptorSetLayout();
  void createGraphicsPipeline();

 private:
  std::vector<char> readShaderFiles(const std::string& filename);
  VkShaderModule createShaderModule(const std::vector<char>& code);
};
inline Pipelines pipelines;
