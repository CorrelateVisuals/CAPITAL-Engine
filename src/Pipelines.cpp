#include <vulkan/vulkan.h>

#include <array>
#include <cstring>
#include <random>

#include "CapitalEngine.h"
#include "Control.h"
#include "Mechanics.h"
#include "Memory.h"
#include "Pipelines.h"
#include "World.h"

Pipelines::Pipelines() : graphics{}, compute{} {
  _log.console("{ PIP }", "constructing Pipelines");
}

Pipelines::~Pipelines() {
  _log.console("{ PIP }", "destructing Pipelines");
}

void Pipelines::createColorResources() {
  VkFormat colorFormat = _mechanics.swapChain.imageFormat;

  _memory.createImage(_mechanics.swapChain.extent.width,
                      _mechanics.swapChain.extent.height, graphics.msaa.samples,
                      colorFormat, VK_IMAGE_TILING_OPTIMAL,
                      VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                      graphics.msaa.colorImage, graphics.msaa.colorImageMemory);
  graphics.msaa.colorImageView = _mechanics.createImageView(
      graphics.msaa.colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Pipelines::createDepthResources() {
  VkFormat depthFormat = findDepthFormat();

  _memory.createImage(_mechanics.swapChain.extent.width,
                      _mechanics.swapChain.extent.height, graphics.msaa.samples,
                      depthFormat, VK_IMAGE_TILING_OPTIMAL,
                      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, graphics.depth.image,
                      graphics.depth.imageMemory);
  graphics.depth.imageView = _mechanics.createImageView(
      graphics.depth.image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Pipelines::createRenderPass() {
  _log.console("{ []< }", "creating Render Pass");
  VkAttachmentDescription colorAttachment{
      .format = _mechanics.swapChain.imageFormat,
      .samples = graphics.msaa.samples,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

  VkAttachmentDescription depthAttachment{
      .format = findDepthFormat(),
      .samples = graphics.msaa.samples,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

  VkAttachmentDescription colorAttachmentResolve{
      .format = _mechanics.swapChain.imageFormat,
      .samples = VK_SAMPLE_COUNT_1_BIT,
      .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

  VkAttachmentReference colorAttachmentRef{
      .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

  VkAttachmentReference depthAttachmentRef{
      .attachment = 1,
      .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

  VkAttachmentReference colorAttachmentResolveRef{
      .attachment = 2, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

  VkSubpassDescription subpass{
      .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachmentRef,
      .pResolveAttachments = &colorAttachmentResolveRef,
      .pDepthStencilAttachment = &depthAttachmentRef};

  VkSubpassDependency dependency{
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                      VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
      .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                      VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
      .srcAccessMask = 0,
      .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                       VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT};

  std::vector<VkAttachmentDescription> attachments = {
      colorAttachment, depthAttachment, colorAttachmentResolve};

  VkRenderPassCreateInfo renderPassInfo{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = static_cast<uint32_t>(attachments.size()),
      .pAttachments = attachments.data(),
      .subpassCount = 1,
      .pSubpasses = &subpass,
      .dependencyCount = 1,
      .pDependencies = &dependency};

  _mechanics.result(vkCreateRenderPass, _mechanics.mainDevice.logical,
                    &renderPassInfo, nullptr, &graphics.renderPass);
}

void Pipelines::createGraphicsPipeline() {
  _log.console("{ PIP }", "creating Graphics Pipeline");

  std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
      getShaderStageInfo(VK_SHADER_STAGE_VERTEX_BIT, "vert.spv", graphics),
      getShaderStageInfo(VK_SHADER_STAGE_FRAGMENT_BIT, "frag.spv", graphics)};

  VkPipelineVertexInputStateCreateInfo vertexInputInfo = getVertexInputInfo();

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      .primitiveRestartEnable = VK_FALSE};

  VkPipelineViewportStateCreateInfo viewportState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .scissorCount = 1};

  VkPipelineRasterizationStateCreateInfo rasterizer{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .depthClampEnable = VK_TRUE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_BACK_BIT,
      .frontFace = VK_FRONT_FACE_CLOCKWISE,
      .depthBiasEnable = VK_TRUE,
      .depthBiasConstantFactor = 0.0f,
      .depthBiasClamp = 0.0f,
      .depthBiasSlopeFactor = 0.0f,
      .lineWidth = 1.0f};

  VkPipelineMultisampleStateCreateInfo multisampling{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = graphics.msaa.samples,
      .sampleShadingEnable = VK_TRUE,
      .minSampleShading = 0.2f};

  VkPipelineDepthStencilStateCreateInfo depthStencil = getDepthStencilInfo();
  VkPipelineColorBlendStateCreateInfo colorBlending = getColorBlendingInfo();
  VkPipelineDynamicStateCreateInfo dynamicState = getDynamicStateInfo();

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &_memory.descriptor.setLayout};

  _mechanics.result(vkCreatePipelineLayout, _mechanics.mainDevice.logical,
                    &pipelineLayoutInfo, nullptr, &graphics.pipelineLayout);

  VkGraphicsPipelineCreateInfo pipelineInfo{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = static_cast<uint32_t>(shaderStages.size()),
      .pStages = shaderStages.data(),
      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &inputAssembly,
      .pViewportState = &viewportState,
      .pRasterizationState = &rasterizer,
      .pMultisampleState = &multisampling,
      .pDepthStencilState = &depthStencil,
      .pColorBlendState = &colorBlending,
      .pDynamicState = &dynamicState,
      .layout = graphics.pipelineLayout,
      .renderPass = graphics.renderPass,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE};

  _mechanics.result(vkCreateGraphicsPipelines, _mechanics.mainDevice.logical,
                    VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                    &graphics.pipeline);

  destroyShaderModules(graphics.shaderModules);
}

VkFormat Pipelines::findSupportedFormat(const std::vector<VkFormat>& candidates,
                                        VkImageTiling tiling,
                                        VkFormatFeatureFlags features) {
  for (VkFormat format : candidates) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(_mechanics.mainDevice.physical, format,
                                        &props);

    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & features) == features) {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
               (props.optimalTilingFeatures & features) == features) {
      return format;
    }
  }

  throw std::runtime_error("\n!ERROR! failed to find supported format!");
}

VkFormat Pipelines::findDepthFormat() {
  return findSupportedFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
       VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool Pipelines::hasStencilComponent(VkFormat format) {
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
         format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkPipelineShaderStageCreateInfo Pipelines::getShaderStageInfo(
    VkShaderStageFlagBits shaderStage,
    std::string shaderName,
    auto pipeline) {
  std::string directory = "shaders/";
  std::string shaderPath = directory + shaderName;

  auto shaderCode = readShaderFile(shaderPath);
  VkShaderModule shaderModule = createShaderModule(shaderCode);
  pipeline.shaderModules.push_back(shaderModule);

  VkPipelineShaderStageCreateInfo shaderStageInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = shaderStage,
      .module = shaderModule,
      .pName = "main"};

  return shaderStageInfo;
}

std::vector<char> Pipelines::readShaderFile(const std::string& filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("\n!ERROR! failed to open file!");
  }

  size_t fileSize = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();

  return buffer;
}

void Pipelines::createComputePipeline() {
  _log.console("{ PIP }", "creating Compute Pipeline");

  VkPipelineShaderStageCreateInfo computeShaderStageInfo =
      getShaderStageInfo(VK_SHADER_STAGE_COMPUTE_BIT, "comp.spv", compute);

  VkPushConstantRange pushConstantRange = {
      .stageFlags = _memory.pushConstants.shaderStage,
      .offset = _memory.pushConstants.offset,
      .size = _memory.pushConstants.size};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &_memory.descriptor.setLayout,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &pushConstantRange};

  _mechanics.result(vkCreatePipelineLayout, _mechanics.mainDevice.logical,
                    &pipelineLayoutInfo, nullptr, &compute.pipelineLayout);

  VkComputePipelineCreateInfo pipelineInfo{
      .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .stage = computeShaderStageInfo,
      .layout = compute.pipelineLayout};

  _mechanics.result(vkCreateComputePipelines, _mechanics.mainDevice.logical,
                    VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                    &compute.pipeline);

  destroyShaderModules(compute.shaderModules);
}

VkSampleCountFlagBits Pipelines::getMaxUsableSampleCount() {
  VkPhysicalDeviceProperties physicalDeviceProperties;
  vkGetPhysicalDeviceProperties(_mechanics.mainDevice.physical,
                                &physicalDeviceProperties);
  VkSampleCountFlags counts =
      physicalDeviceProperties.limits.framebufferColorSampleCounts &
      physicalDeviceProperties.limits.framebufferDepthSampleCounts;

  for (int i = VK_SAMPLE_COUNT_64_BIT; i >= VK_SAMPLE_COUNT_1_BIT; i >>= 1) {
    if (counts & i) {
      return static_cast<VkSampleCountFlagBits>(i);
    }
  }
  return VK_SAMPLE_COUNT_1_BIT;
}

VkShaderModule Pipelines::createShaderModule(const std::vector<char>& code) {
  _log.console(_log.style.charLeader, "creating Shader Module");
  VkShaderModuleCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = code.size(),
      .pCode = reinterpret_cast<const uint32_t*>(code.data())};

  VkShaderModule shaderModule;

  _mechanics.result(vkCreateShaderModule, _mechanics.mainDevice.logical,
                    &createInfo, nullptr, &shaderModule);

  return shaderModule;
}

void Pipelines::destroyShaderModules(
    std::vector<VkShaderModule>& shaderModules) {
  for (size_t i = 0; i < shaderModules.size(); i++) {
    vkDestroyShaderModule(_mechanics.mainDevice.logical, shaderModules[i],
                          nullptr);
  }
};

VkPipelineVertexInputStateCreateInfo Pipelines::getVertexInputInfo() {
  static auto bindingDescriptions = World::getBindingDescriptions();
  static auto attributeDescriptions = World::getAttributeDescriptions();

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .vertexBindingDescriptionCount =
          static_cast<uint32_t>(bindingDescriptions.size()),
      .pVertexBindingDescriptions = bindingDescriptions.data(),
      .vertexAttributeDescriptionCount =
          static_cast<uint32_t>(attributeDescriptions.size()),
      .pVertexAttributeDescriptions = attributeDescriptions.data()};

  return vertexInputInfo;
}

VkPipelineColorBlendStateCreateInfo Pipelines::getColorBlendingInfo() {
  static VkPipelineColorBlendAttachmentState colorBlendAttachment{
      .blendEnable = VK_FALSE,
      .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
      .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
      .colorBlendOp = VK_BLEND_OP_ADD,
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
      .alphaBlendOp = VK_BLEND_OP_ADD,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};

  VkPipelineColorBlendStateCreateInfo colorBlending{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .logicOpEnable = VK_FALSE,
      .logicOp = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment,
      .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}};
  return colorBlending;
}

VkPipelineDynamicStateCreateInfo Pipelines::getDynamicStateInfo() {
  static std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
                                                      VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicState{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates = dynamicStates.data()};
  return dynamicState;
}

VkPipelineDepthStencilStateCreateInfo Pipelines::getDepthStencilInfo() {
  VkPipelineDepthStencilStateCreateInfo depthStencil{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .depthTestEnable = VK_TRUE,
      .depthWriteEnable = VK_TRUE,
      .depthCompareOp = VK_COMPARE_OP_LESS,
      .depthBoundsTestEnable = VK_FALSE,
      .stencilTestEnable = VK_FALSE};
  return depthStencil;
};
