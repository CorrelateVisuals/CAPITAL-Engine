#include <vulkan/vulkan.h>

#include <array>
#include <cstring>
#include <random>

#include "CapitalEngine.h"
#include "Control.h"
#include "Mechanics.h"
#include "Pipelines.h"
#include "World.h"

Pipelines::Pipelines() : graphics{}, compute{} {
  _log.console("{ PIP }", "constructing Pipelines");
}

Pipelines::~Pipelines() {
  _log.console("{ PIP }", "destructing Pipelines");
}

MemoryCommands::MemoryCommands()
    : command{}, uniform{}, shaderStorage{}, descriptor{} {
  _log.console("{ 010 }", "constructing Memory Management");
}

MemoryCommands::~MemoryCommands() {
  _log.console("{ 010 }", "destructing Memory Management");
}

void Pipelines::createColorResources() {
  VkFormat colorFormat = _mechanics.swapChain.imageFormat;

  _memCommands.createImage(_mechanics.swapChain.extent.width,
                           _mechanics.swapChain.extent.height, msaa.samples,
                           colorFormat, VK_IMAGE_TILING_OPTIMAL,
                           VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, msaa.colorImage,
                           msaa.colorImageMemory);
  msaa.colorImageView = _mechanics.createImageView(msaa.colorImage, colorFormat,
                                                   VK_IMAGE_ASPECT_COLOR_BIT);
}

void Pipelines::createDepthResources() {
  VkFormat depthFormat = findDepthFormat();

  _memCommands.createImage(
      _mechanics.swapChain.extent.width, _mechanics.swapChain.extent.height,
      msaa.samples, depthFormat, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth.image, depth.imageMemory);
  depth.imageView = _mechanics.createImageView(depth.image, depthFormat,
                                               VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Pipelines::createRenderPass() {
  _log.console("{ []< }", "creating Render Pass");
  VkAttachmentDescription colorAttachment{
      .format = _mechanics.swapChain.imageFormat,
      .samples = msaa.samples,
      .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
      .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
      .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

  VkAttachmentDescription depthAttachment{
      .format = findDepthFormat(),
      .samples = msaa.samples,
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

  if (vkCreateRenderPass(_mechanics.mainDevice.logical, &renderPassInfo,
                         nullptr, &renderPass) != VK_SUCCESS) {
    throw std::runtime_error("!ERROR! failed to create render pass!");
  }
}

void MemoryCommands::createDescriptorSetLayout() {
  _log.console("{ DES }", "creating Compute Descriptor Set Layout");

  std::vector<VkDescriptorSetLayoutBinding> layoutBindings = {
      {.binding = 0,
       .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
       .descriptorCount = 1,
       .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT,
       .pImmutableSamplers = nullptr},
      {.binding = 1,
       .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
       .descriptorCount = 1,
       .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
       .pImmutableSamplers = nullptr},
      {.binding = 2,
       .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
       .descriptorCount = 1,
       .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
       .pImmutableSamplers = nullptr}};

  VkDescriptorSetLayoutCreateInfo layoutInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = static_cast<uint32_t>(layoutBindings.size()),
      .pBindings = layoutBindings.data()};

  if (vkCreateDescriptorSetLayout(
          _mechanics.mainDevice.logical, &layoutInfo, nullptr,
          &_memCommands.descriptor.setLayout) != VK_SUCCESS) {
    throw std::runtime_error(
        "!ERROR! failed to create compute descriptor set layout!");
  }
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
      .depthClampEnable = VK_FALSE,
      .rasterizerDiscardEnable = VK_FALSE,
      .polygonMode = VK_POLYGON_MODE_FILL,
      .cullMode = VK_CULL_MODE_NONE,
      .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .depthBiasEnable = VK_FALSE,
      .lineWidth = 1.0f};

  VkPipelineMultisampleStateCreateInfo multisampling{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = msaa.samples,
      .sampleShadingEnable = VK_TRUE,
      .minSampleShading = 0.2f};

  VkPipelineDepthStencilStateCreateInfo depthStencil = getDepthStencilInfo();
  VkPipelineColorBlendStateCreateInfo colorBlending = getColorBlendingInfo();
  VkPipelineDynamicStateCreateInfo dynamicState = getDynamicStateInfo();

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &_memCommands.descriptor.setLayout};

  if (vkCreatePipelineLayout(_mechanics.mainDevice.logical, &pipelineLayoutInfo,
                             nullptr, &graphics.pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("!ERROR! failed to create pipeline layout!");
  }

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
      .renderPass = renderPass,
      .subpass = 0,
      .basePipelineHandle = VK_NULL_HANDLE};

  if (vkCreateGraphicsPipelines(_mechanics.mainDevice.logical, VK_NULL_HANDLE,
                                1, &pipelineInfo, nullptr,
                                &graphics.pipeline) != VK_SUCCESS) {
    throw std::runtime_error("!ERROR! failed to create graphics pipeline!");
  }

  for (size_t i = 0; i < graphics.shaderModules.size(); i++) {
    vkDestroyShaderModule(_mechanics.mainDevice.logical,
                          graphics.shaderModules[i], nullptr);
  }
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

  throw std::runtime_error("!ERROR! failed to find supported format!");
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
    throw std::runtime_error("!ERROR! failed to open file!");
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
      .stageFlags = _memCommands.pushConstants.shaderStage,
      .offset = _memCommands.pushConstants.offset,
      .size = _memCommands.pushConstants.size};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &_memCommands.descriptor.setLayout,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &pushConstantRange};

  if (vkCreatePipelineLayout(_mechanics.mainDevice.logical, &pipelineLayoutInfo,
                             nullptr, &compute.pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error(
        "!ERROR! failed to create compute pipeline layout!");
  }

  VkComputePipelineCreateInfo pipelineInfo{
      .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .stage = computeShaderStageInfo,
      .layout = compute.pipelineLayout};

  if (vkCreateComputePipelines(_mechanics.mainDevice.logical, VK_NULL_HANDLE, 1,
                               &pipelineInfo, nullptr,
                               &compute.pipeline) != VK_SUCCESS) {
    throw std::runtime_error("!ERROR! failed to create compute pipeline!");
  }

  for (size_t i = 0; i < compute.shaderModules.size(); i++) {
    vkDestroyShaderModule(_mechanics.mainDevice.logical,
                          compute.shaderModules[i], nullptr);
  }
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
  if (vkCreateShaderModule(_mechanics.mainDevice.logical, &createInfo, nullptr,
                           &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("!ERROR! failed to create shader module!");
  }

  return shaderModule;
}

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

void MemoryCommands::createFramebuffers() {
  _log.console("{ BUF }", "creating Frame Buffers");

  _mechanics.swapChain.framebuffers.resize(
      _mechanics.swapChain.imageViews.size());

  for (size_t i = 0; i < _mechanics.swapChain.imageViews.size(); i++) {
    std::array<VkImageView, 3> attachments = {
        _pipelines.msaa.colorImageView, _pipelines.depth.imageView,
        _mechanics.swapChain.imageViews[i]};

    VkFramebufferCreateInfo framebufferInfo{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = _pipelines.renderPass,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .width = _mechanics.swapChain.extent.width,
        .height = _mechanics.swapChain.extent.height,
        .layers = 1};

    if (vkCreateFramebuffer(_mechanics.mainDevice.logical, &framebufferInfo,
                            nullptr, &_mechanics.swapChain.framebuffers[i]) !=
        VK_SUCCESS) {
      throw std::runtime_error("!ERROR! failed to create framebuffer!");
    }
  }
}

void MemoryCommands::createCommandPool() {
  _log.console("{ CMD }", "creating Command Pool");

  VulkanMechanics::Queues::FamilyIndices queueFamilyIndices =
      _mechanics.findQueueFamilies(_mechanics.mainDevice.physical);

  VkCommandPoolCreateInfo poolInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = queueFamilyIndices.graphicsAndComputeFamily.value()};

  if (vkCreateCommandPool(_mechanics.mainDevice.logical, &poolInfo, nullptr,
                          &command.pool) != VK_SUCCESS) {
    throw std::runtime_error("!ERROR! failed to create graphics command pool!");
  }
}

void MemoryCommands::createCommandBuffers() {
  _log.console("{ CMD }", "creating Command Buffers");

  command.graphicBuffers.resize(MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocateInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = command.pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount =
          static_cast<uint32_t>(command.graphicBuffers.size())};

  if (vkAllocateCommandBuffers(_mechanics.mainDevice.logical, &allocateInfo,
                               command.graphicBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("!ERROR! failed to allocate command buffers!");
  }
}

void MemoryCommands::createComputeCommandBuffers() {
  _log.console("{ CMD }", "creating Compute Command Buffers");

  command.computeBuffers.resize(MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocateInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = command.pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount =
          static_cast<uint32_t>(command.computeBuffers.size())};

  if (vkAllocateCommandBuffers(_mechanics.mainDevice.logical, &allocateInfo,
                               command.computeBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error(
        "!ERROR! failed to allocate compute command buffers!");
  }
}

void MemoryCommands::createShaderStorageBuffers() {
  _log.console("{ BUF }", "creating Shader Storage Buffers");

  std::vector<World::Cell> cells = _world.initializeCells();

  VkDeviceSize bufferSize = sizeof(World::Cell) * _control.grid.dimensions[0] *
                            _control.grid.dimensions[1];

  // Create a staging buffer used to upload data to the gpu
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               stagingBuffer, stagingBufferMemory);

  void* data;
  vkMapMemory(_mechanics.mainDevice.logical, stagingBufferMemory, 0, bufferSize,
              0, &data);
  std::memcpy(data, cells.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(_mechanics.mainDevice.logical, stagingBufferMemory);

  shaderStorage.buffers.resize(MAX_FRAMES_IN_FLIGHT);
  shaderStorage.buffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

  // Copy initial Cell data to all storage buffers
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    createBuffer(static_cast<VkDeviceSize>(bufferSize),
                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, shaderStorage.buffers[i],
                 shaderStorage.buffersMemory[i]);
    copyBuffer(stagingBuffer, shaderStorage.buffers[i], bufferSize);
  }

  vkDestroyBuffer(_mechanics.mainDevice.logical, stagingBuffer, nullptr);
  vkFreeMemory(_mechanics.mainDevice.logical, stagingBufferMemory, nullptr);
}

void MemoryCommands::createUniformBuffers() {
  _log.console("{ BUF }", "creating Uniform Buffers");
  VkDeviceSize bufferSize = sizeof(World::UniformBufferObject);

  uniform.buffers.resize(MAX_FRAMES_IN_FLIGHT);
  uniform.buffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
  uniform.buffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 uniform.buffers[i], uniform.buffersMemory[i]);

    vkMapMemory(_mechanics.mainDevice.logical, uniform.buffersMemory[i], 0,
                bufferSize, 0, &uniform.buffersMapped[i]);
  }
}

void MemoryCommands::createDescriptorPool() {
  _log.console("{ DES }", "creating Descriptor Pools");
  std::vector<VkDescriptorPoolSize> poolSizes{
      {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
       .descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)},
      {.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
       .descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * 2}};

  VkDescriptorPoolCreateInfo poolInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
      .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
      .pPoolSizes = poolSizes.data()};

  if (vkCreateDescriptorPool(_mechanics.mainDevice.logical, &poolInfo, nullptr,
                             &_memCommands.descriptor.pool) != VK_SUCCESS) {
    throw std::runtime_error("!ERROR! failed to create descriptor pool!");
  }
}

void MemoryCommands::createImage(uint32_t width,
                                 uint32_t height,
                                 VkSampleCountFlagBits numSamples,
                                 VkFormat format,
                                 VkImageTiling tiling,
                                 VkImageUsageFlags usage,
                                 VkMemoryPropertyFlags properties,
                                 VkImage& image,
                                 VkDeviceMemory& imageMemory) {
  VkImageCreateInfo imageInfo{
      .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .imageType = VK_IMAGE_TYPE_2D,
      .format = format,
      .extent = {.width = width, .height = height, .depth = 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = numSamples,
      .tiling = tiling,
      .usage = usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
      .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED};

  if (vkCreateImage(_mechanics.mainDevice.logical, &imageInfo, nullptr,
                    &image) != VK_SUCCESS) {
    throw std::runtime_error("!ERROR! failed to create image!");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(_mechanics.mainDevice.logical, image,
                               &memRequirements);

  VkMemoryAllocateInfo allocateInfo{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memRequirements.size,
      .memoryTypeIndex =
          findMemoryType(memRequirements.memoryTypeBits, properties)};

  if (vkAllocateMemory(_mechanics.mainDevice.logical, &allocateInfo, nullptr,
                       &imageMemory) != VK_SUCCESS) {
    throw std::runtime_error("!ERROR! failed to allocate image memory!");
  }
  vkBindImageMemory(_mechanics.mainDevice.logical, image, imageMemory, 0);
}

void MemoryCommands::createDescriptorSets() {
  _log.console("{ DES }", "creating Compute Descriptor Sets");
  std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
                                             descriptor.setLayout);
  VkDescriptorSetAllocateInfo allocateInfo{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = descriptor.pool,
      .descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
      .pSetLayouts = layouts.data()};

  descriptor.sets.resize(MAX_FRAMES_IN_FLIGHT);
  if (vkAllocateDescriptorSets(_mechanics.mainDevice.logical, &allocateInfo,
                               descriptor.sets.data()) != VK_SUCCESS) {
    throw std::runtime_error("!ERROR! failed to allocate descriptor sets!");
  }

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    VkDescriptorBufferInfo uniformBufferInfo{
        .buffer = uniform.buffers[i],
        .offset = 0,
        .range = sizeof(World::UniformBufferObject)};

    VkDescriptorBufferInfo storageBufferInfoLastFrame{
        .buffer = shaderStorage.buffers[(i - 1) % MAX_FRAMES_IN_FLIGHT],
        .offset = 0,
        .range = sizeof(World::Cell) * _control.grid.dimensions[0] *
                 _control.grid.dimensions[1]};

    VkDescriptorBufferInfo storageBufferInfoCurrentFrame{
        .buffer = shaderStorage.buffers[i],
        .offset = 0,
        .range = sizeof(World::Cell) * _control.grid.dimensions[0] *
                 _control.grid.dimensions[1]};

    std::vector<VkWriteDescriptorSet> descriptorWrites{
        {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
         .dstSet = descriptor.sets[i],
         .dstBinding = 0,
         .dstArrayElement = 0,
         .descriptorCount = 1,
         .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
         .pBufferInfo = &uniformBufferInfo},

        {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
         .dstSet = descriptor.sets[i],
         .dstBinding = 1,
         .dstArrayElement = 0,
         .descriptorCount = 1,
         .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
         .pBufferInfo = &storageBufferInfoLastFrame},

        {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
         .dstSet = descriptor.sets[i],
         .dstBinding = 2,
         .dstArrayElement = 0,
         .descriptorCount = 1,
         .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
         .pBufferInfo = &storageBufferInfoCurrentFrame}};

    vkUpdateDescriptorSets(_mechanics.mainDevice.logical,
                           static_cast<uint32_t>(descriptorWrites.size()),
                           descriptorWrites.data(), 0, nullptr);
  }
}

void MemoryCommands::updateUniformBuffer(uint32_t currentImage) {
  World::UniformBufferObject uniformObject = _world.updateUniforms();
  std::memcpy(uniform.buffersMapped[currentImage], &uniformObject,
              sizeof(uniformObject));
}

void MemoryCommands::recordComputeCommandBuffer(VkCommandBuffer commandBuffer) {
  VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error(
        "failed to begin recording compute command buffer!");
  }

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                    _pipelines.compute.pipeline);

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                          _pipelines.compute.pipelineLayout, 0, 1,
                          &descriptor.sets[_mechanics.syncObjects.currentFrame],
                          0, nullptr);

  _control.setPushConstants();
  vkCmdPushConstants(commandBuffer, _pipelines.compute.pipelineLayout,
                     pushConstants.shaderStage, pushConstants.offset,
                     pushConstants.size, pushConstants.data.data());

  uint32_t numberOfWorkgroupsX =
      (_control.grid.dimensions[0] + _control.compute.localSizeX - 1) /
      _control.compute.localSizeX;
  uint32_t numberOfWorkgroupsY =
      (_control.grid.dimensions[1] + _control.compute.localSizeY - 1) /
      _control.compute.localSizeY;

  vkCmdDispatch(commandBuffer, numberOfWorkgroupsX, numberOfWorkgroupsY, 1);

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error(
        "!ERROR! failed to record compute command buffer!");
  }
}

void MemoryCommands::recordCommandBuffer(VkCommandBuffer commandBuffer,
                                         uint32_t imageIndex) {
  VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error(
        "!ERROR! failed to begin recording command buffer!");
  }
  std::vector<VkClearValue> clearValues{{.color = {{0.0f, 0.0f, 0.0f, 1.0f}}},
                                        {.depthStencil = {1.0f, 0}}};

  VkRenderPassBeginInfo renderPassInfo{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .pNext = nullptr,
      .renderPass = _pipelines.renderPass,
      .framebuffer = _mechanics.swapChain.framebuffers[imageIndex],
      .renderArea = {.offset = {0, 0}, .extent = _mechanics.swapChain.extent},
      .clearValueCount = static_cast<uint32_t>(clearValues.size()),
      .pClearValues = clearValues.data()};

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    _pipelines.graphics.pipeline);

  VkViewport viewport{
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(_mechanics.swapChain.extent.width),
      .height = static_cast<float>(_mechanics.swapChain.extent.height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f};
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{.offset = {0, 0}, .extent = _mechanics.swapChain.extent};
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  VkDeviceSize offsets[]{0};
  vkCmdBindVertexBuffers(
      commandBuffer, 0, 1,
      &_memCommands.shaderStorage.buffers[_mechanics.syncObjects.currentFrame],
      offsets);

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          _pipelines.graphics.pipelineLayout, 0, 1,
                          &descriptor.sets[_mechanics.syncObjects.currentFrame],
                          0, nullptr);

  vkCmdDraw(commandBuffer, 36,
            _control.grid.dimensions[0] * _control.grid.dimensions[1], 0, 0);

  vkCmdEndRenderPass(commandBuffer);

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("!ERROR! failed to record command buffer!");
  }
}

void MemoryCommands::createBuffer(VkDeviceSize size,
                                  VkBufferUsageFlags usage,
                                  VkMemoryPropertyFlags properties,
                                  VkBuffer& buffer,
                                  VkDeviceMemory& bufferMemory) {
  VkBufferCreateInfo bufferInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                .size = size,
                                .usage = usage,
                                .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

  _log.console("{ ... }",
               "creating Buffer:", _log.getBufferUsageString(bufferInfo.usage));
  _log.console(_log.style.charLeader, bufferInfo.size, "bytes");

  if (vkCreateBuffer(_mechanics.mainDevice.logical, &bufferInfo, nullptr,
                     &buffer) != VK_SUCCESS) {
    throw std::runtime_error("!ERROR! failed to create buffer!");
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(_mechanics.mainDevice.logical, buffer,
                                &memRequirements);

  VkMemoryAllocateInfo allocateInfo{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memRequirements.size,
      .memoryTypeIndex =
          findMemoryType(memRequirements.memoryTypeBits, properties)};

  if (vkAllocateMemory(_mechanics.mainDevice.logical, &allocateInfo, nullptr,
                       &bufferMemory) != VK_SUCCESS) {
    throw std::runtime_error("!ERROR! failed to allocate buffer memory!");
  }

  vkBindBufferMemory(_mechanics.mainDevice.logical, buffer, bufferMemory, 0);
}

void MemoryCommands::copyBuffer(VkBuffer srcBuffer,
                                VkBuffer dstBuffer,
                                VkDeviceSize size) {
  VkCommandBufferAllocateInfo allocateInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = command.pool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1};

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(_mechanics.mainDevice.logical, &allocateInfo,
                           &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  VkBufferCopy copyRegion{.size = size};
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                          .commandBufferCount = 1,
                          .pCommandBuffers = &commandBuffer};

  vkQueueSubmit(_mechanics.queues.graphics, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(_mechanics.queues.graphics);

  vkFreeCommandBuffers(_mechanics.mainDevice.logical, command.pool, 1,
                       &commandBuffer);
}

uint32_t MemoryCommands::findMemoryType(uint32_t typeFilter,
                                        VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(_mechanics.mainDevice.physical,
                                      &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags &
                                    properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("!ERROR! failed to find suitable memory type!");
}
