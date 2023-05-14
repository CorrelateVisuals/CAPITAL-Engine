#include <vulkan/vulkan.h>

#include <array>
#include <cstring>
#include <random>

#include "CapitalEngine.h"
#include "Mechanics.h"
#include "Pipelines.h"
#include "World.h"

Pipelines::Pipelines() : graphics{}, compute{} {
  _log.console("{ PIP }", "constructing Pipelines");
}

Pipelines::~Pipelines() {
  _log.console("{ PIP }", "destructing Pipelines");
}

void Pipelines::createDepthResources() {
  VkFormat depthFormat = findDepthFormat();

  _memCommands.createImage(
      _mechanics.swapChain.extent.width, _mechanics.swapChain.extent.height,
      depthFormat, VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth.image, depth.imageMemory);
  depth.imageView = _mechanics.createImageView(depth.image, depthFormat,
                                               VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Pipelines::createRenderPass() {
  _log.console("{ []< }", "creating Render Pass");
  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = _mechanics.swapChain.imageFormat;
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentDescription depthAttachment{};
  depthAttachment.format = findDepthFormat();
  depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout =
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthAttachmentRef{};
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;
  subpass.pDepthStencilAttachment = &depthAttachmentRef;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                             VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  std::array<VkAttachmentDescription, 2> attachments = {colorAttachment,
                                                        depthAttachment};
  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  if (vkCreateRenderPass(_mechanics.mainDevice.logical, &renderPassInfo,
                         nullptr, &renderPass) != VK_SUCCESS) {
    throw std::runtime_error("failed to create render pass!");
  }
}

void MemoryCommands::createComputeDescriptorSetLayout() {
  _log.console("{ DES }", "creating Compute Descriptor Set Layout");

  std::array<VkDescriptorSetLayoutBinding, 3> layoutBindings{};
  layoutBindings[0].binding = 0;
  layoutBindings[0].descriptorCount = 1;
  layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  layoutBindings[0].pImmutableSamplers = nullptr;
  layoutBindings[0].stageFlags =
      VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT;

  layoutBindings[1].binding = 1;
  layoutBindings[1].descriptorCount = 1;
  layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  layoutBindings[1].pImmutableSamplers = nullptr;
  layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

  layoutBindings[2].binding = 2;
  layoutBindings[2].descriptorCount = 1;
  layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  layoutBindings[2].pImmutableSamplers = nullptr;
  layoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = 3;
  layoutInfo.pBindings = layoutBindings.data();

  if (vkCreateDescriptorSetLayout(
          _mechanics.mainDevice.logical, &layoutInfo, nullptr,
          &_memCommands.descriptor.setLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create compute descriptor set layout!");
  }
}

void Pipelines::createGraphicsPipeline() {
  _log.console("{ PIP }", "creating Graphics Pipeline");
  auto vertShaderCode = readShaderFile("shaders/vert.spv");
  auto fragShaderCode = readShaderFile("shaders/frag.spv");

  VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
  VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

  VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
  vertShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
  fragShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                    fragShaderStageInfo};

  VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
  vertexInputInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  auto bindingDescriptions = World::getBindingDescriptions();
  auto attributeDescriptions = World::getAttributeDescriptions();

  vertexInputInfo.vertexBindingDescriptionCount =
      static_cast<uint32_t>(bindingDescriptions.size());
  vertexInputInfo.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

  VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
  inputAssembly.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo viewportState{};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  VkPipelineRasterizationStateCreateInfo rasterizer{};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_NONE;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisampling{};
  multisampling.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineDepthStencilStateCreateInfo depthStencil{};
  depthStencil.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencil.depthTestEnable = VK_TRUE;
  depthStencil.depthWriteEnable = VK_TRUE;
  depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
  depthStencil.depthBoundsTestEnable = VK_FALSE;
  depthStencil.stencilTestEnable = VK_FALSE;

  VkPipelineColorBlendAttachmentState colorBlendAttachment{};
  colorBlendAttachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;
  colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  colorBlendAttachment.dstColorBlendFactor =
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
  colorBlendAttachment.srcAlphaBlendFactor =
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

  VkPipelineColorBlendStateCreateInfo colorBlending{};
  colorBlending.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;

  std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
                                               VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicState{};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamicState.pDynamicStates = dynamicStates.data();

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &_memCommands.descriptor.setLayout;

  if (vkCreatePipelineLayout(_mechanics.mainDevice.logical, &pipelineLayoutInfo,
                             nullptr, &graphics.pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }

  VkGraphicsPipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pDepthStencilState = &depthStencil;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.pDynamicState = &dynamicState;
  pipelineInfo.layout = graphics.pipelineLayout;
  pipelineInfo.renderPass = renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  if (vkCreateGraphicsPipelines(_mechanics.mainDevice.logical, VK_NULL_HANDLE,
                                1, &pipelineInfo, nullptr,
                                &graphics.pipeline) != VK_SUCCESS) {
    throw std::runtime_error("failed to create graphics pipeline!");
  }

  vkDestroyShaderModule(_mechanics.mainDevice.logical, fragShaderModule,
                        nullptr);
  vkDestroyShaderModule(_mechanics.mainDevice.logical, vertShaderModule,
                        nullptr);
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

  throw std::runtime_error("failed to find supported format!");
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

std::vector<char> Pipelines::readShaderFile(const std::string& filename) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("failed to open file!");
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
  auto computeShaderCode = readShaderFile("shaders/comp.spv");

  VkShaderModule computeShaderModule = createShaderModule(computeShaderCode);

  VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
  computeShaderStageInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  computeShaderStageInfo.module = computeShaderModule;
  computeShaderStageInfo.pName = "main";

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &_memCommands.descriptor.setLayout;

  if (vkCreatePipelineLayout(_mechanics.mainDevice.logical, &pipelineLayoutInfo,
                             nullptr, &compute.pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create compute pipeline layout!");
  }

  VkComputePipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  pipelineInfo.layout = compute.pipelineLayout;
  pipelineInfo.stage = computeShaderStageInfo;

  if (vkCreateComputePipelines(_mechanics.mainDevice.logical, VK_NULL_HANDLE, 1,
                               &pipelineInfo, nullptr,
                               &compute.pipeline) != VK_SUCCESS) {
    throw std::runtime_error("failed to create compute pipeline!");
  }

  vkDestroyShaderModule(_mechanics.mainDevice.logical, computeShaderModule,
                        nullptr);
}

VkShaderModule Pipelines::createShaderModule(const std::vector<char>& code) {
  _log.console(_log.style.charLeader, "creating Shader Module");
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(_mechanics.mainDevice.logical, &createInfo, nullptr,
                           &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("failed to create shader module!");
  }

  return shaderModule;
}

MemoryCommands::MemoryCommands()
    : command{}, uniform{}, shaderStorage{}, descriptor{} {
  _log.console("{ 010 }", "constructing Memory Management");
}

MemoryCommands::~MemoryCommands() {
  _log.console("{ 010 }", "destructing Memory Management");
}

void MemoryCommands::createFramebuffers() {
  _log.console("{ BUF }", "creating Frame Buffers");

  _mechanics.swapChain.framebuffers.resize(
      _mechanics.swapChain.imageViews.size());

  for (size_t i = 0; i < _mechanics.swapChain.imageViews.size(); i++) {
    std::array<VkImageView, 2> attachments = {
        _mechanics.swapChain.imageViews[i], _pipelines.depth.imageView};

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = _pipelines.renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = _mechanics.swapChain.extent.width;
    framebufferInfo.height = _mechanics.swapChain.extent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(_mechanics.mainDevice.logical, &framebufferInfo,
                            nullptr, &_mechanics.swapChain.framebuffers[i]) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create framebuffer!");
    }
  }
}

void MemoryCommands::createCommandPool() {
  _log.console("{ CMD }", "creating Command Pool");

  VulkanMechanics::Queues::FamilyIndices queueFamilyIndices =
      _mechanics.findQueueFamilies(_mechanics.mainDevice.physical);

  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex =
      queueFamilyIndices.graphicsAndComputeFamily.value();

  if (vkCreateCommandPool(_mechanics.mainDevice.logical, &poolInfo, nullptr,
                          &command.pool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create graphics command pool!");
  }
}

void MemoryCommands::createCommandBuffers() {
  _log.console("{ CMD }", "creating Command Buffers");

  command.graphicBuffers.resize(MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = command.pool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount =
      static_cast<uint32_t>(command.graphicBuffers.size());

  if (vkAllocateCommandBuffers(_mechanics.mainDevice.logical, &allocInfo,
                               command.graphicBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }
}

void MemoryCommands::createComputeCommandBuffers() {
  _log.console("{ CMD }", "creating Compute Command Buffers");

  command.computeBuffers.resize(MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = command.pool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount =
      static_cast<uint32_t>(command.computeBuffers.size());

  if (vkAllocateCommandBuffers(_mechanics.mainDevice.logical, &allocInfo,
                               command.computeBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate compute command buffers!");
  }
}

void MemoryCommands::createShaderStorageBuffers() {
  _log.console("{ BUF }", "creating Shader Storage Buffers");
  // Initiliazation of cells on the grid
  _log.console(_log.style.charLeader, "initializing Cells");
  std::vector<World::Cell> cells(_control.grid.numberOfGridPoints);
  std::vector<int> aliveCells =
      _control.setCellsAliveRandomly(_control.grid.numberOfAliveCells);

  // Grid size
  const int gridWidth = _control.grid.gridDimensions[0];
  const int gridHeight = _control.grid.gridDimensions[1];
  const float gridPointDistance = _control.grid.gridPointDistance;
  // Grid cell size
  const float cellWidth = gridPointDistance / gridWidth;
  const float cellHeight = gridPointDistance / gridHeight;
  // Cell position offset
  const float remainingWidth = 2.0f - gridPointDistance;
  const float remainingHeight = 2.0f - gridPointDistance;
  const float offsetX = -1.0f + remainingWidth / 2.0f + cellWidth / 2.0f;
  const float offsetY = -1.0f + remainingHeight / 2.0f + cellHeight / 2.0f;

  // Initialize cells on grid
  for (int x = 0; x < gridWidth; x++) {
    for (int y = 0; y < gridHeight; y++) {
      int index = x + y * gridWidth;
      cells[index].position = {offsetX + x * cellWidth,
                               offsetY + y * cellHeight,
                               _control.getRandomFloat(0.0f, 0.3f), 1.0f};
      if (std::find(aliveCells.begin(), aliveCells.end(), index) !=
          aliveCells.end()) {
        cells[index].size = {_control.grid.cellSize, 0.0f, 0.0f, 0.0f};
        cells[index].color = {0.0f, 0.0f, 1.0f, 1.0f};
      } else {
        cells[index].size = {parameterOff, parameterOff, parameterOff,
                             parameterOff};
        cells[index].color = {1.0f, 0.0f, 0.0f, 0.0f};
      }
    }
  }
  _log.console(aliveCells);

  VkDeviceSize bufferSize =
      sizeof(World::Cell) * _control.grid.numberOfGridPoints;

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
  const int numPools = 2;
  _log.console("{ DES }", "creating", numPools, "Descriptor Pools");
  std::array<VkDescriptorPoolSize, numPools> poolSizes{};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

  poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  poolSizes[1].descriptorCount =
      static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * 2;

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = numPools;
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

  if (vkCreateDescriptorPool(_mechanics.mainDevice.logical, &poolInfo, nullptr,
                             &_memCommands.descriptor.pool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

void MemoryCommands::createImage(uint32_t width,
                                 uint32_t height,
                                 VkFormat format,
                                 VkImageTiling tiling,
                                 VkImageUsageFlags usage,
                                 VkMemoryPropertyFlags properties,
                                 VkImage& image,
                                 VkDeviceMemory& imageMemory) {
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateImage(_mechanics.mainDevice.logical, &imageInfo, nullptr,
                    &image) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image!");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(_mechanics.mainDevice.logical, image,
                               &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(_mechanics.mainDevice.logical, &allocInfo, nullptr,
                       &imageMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate image memory!");
  }

  vkBindImageMemory(_mechanics.mainDevice.logical, image, imageMemory, 0);
}

void MemoryCommands::createComputeDescriptorSets() {
  _log.console("{ DES }", "creating Compute Descriptor Sets");
  std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
                                             descriptor.setLayout);
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptor.pool;
  allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
  allocInfo.pSetLayouts = layouts.data();

  descriptor.sets.resize(MAX_FRAMES_IN_FLIGHT);
  if (vkAllocateDescriptorSets(_mechanics.mainDevice.logical, &allocInfo,
                               descriptor.sets.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate descriptor sets!");
  }

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    VkDescriptorBufferInfo uniformBufferInfo{};
    uniformBufferInfo.buffer = uniform.buffers[i];
    uniformBufferInfo.offset = 0;
    uniformBufferInfo.range = sizeof(World::UniformBufferObject);

    std::array<VkWriteDescriptorSet, 3> descriptorWrites{};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptor.sets[i];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &uniformBufferInfo;

    VkDescriptorBufferInfo storageBufferInfoLastFrame{};
    storageBufferInfoLastFrame.buffer =
        shaderStorage.buffers[(i - 1) % MAX_FRAMES_IN_FLIGHT];
    storageBufferInfoLastFrame.offset = 0;
    storageBufferInfoLastFrame.range =
        sizeof(World::Cell) * _control.grid.numberOfGridPoints;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptor.sets[i];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &storageBufferInfoLastFrame;

    VkDescriptorBufferInfo storageBufferInfoCurrentFrame{};
    storageBufferInfoCurrentFrame.buffer = shaderStorage.buffers[i];
    storageBufferInfoCurrentFrame.offset = 0;
    storageBufferInfoCurrentFrame.range =
        sizeof(World::Cell) * _control.grid.numberOfGridPoints;

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = descriptor.sets[i];
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pBufferInfo = &storageBufferInfoCurrentFrame;

    vkUpdateDescriptorSets(_mechanics.mainDevice.logical, 3,
                           descriptorWrites.data(), 0, nullptr);
  }
}

void MemoryCommands::updateUniformBuffer(uint32_t currentImage) {
  World::UniformBufferObject uniformObject = _world.updateUniforms();
  std::memcpy(uniform.buffersMapped[currentImage], &uniformObject,
              sizeof(uniformObject));
}

void MemoryCommands::recordComputeCommandBuffer(VkCommandBuffer commandBuffer) {
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

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

  vkCmdDispatch(commandBuffer, _control.grid.gridDimensions[0],
                _control.grid.gridDimensions[1], 1);

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record compute command buffer!");
  }
}

void MemoryCommands::recordCommandBuffer(VkCommandBuffer commandBuffer,
                                         uint32_t imageIndex) {
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = _pipelines.renderPass;
  renderPassInfo.framebuffer = _mechanics.swapChain.framebuffers[imageIndex];
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = _mechanics.swapChain.extent;

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
  clearValues[1].depthStencil = {1.0f, 0};

  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    _pipelines.graphics.pipeline);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(_mechanics.swapChain.extent.width);
  viewport.height = static_cast<float>(_mechanics.swapChain.extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = _mechanics.swapChain.extent;
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(
      commandBuffer, 0, 1,
      &_memCommands.shaderStorage.buffers[_mechanics.syncObjects.currentFrame],
      offsets);

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          _pipelines.graphics.pipelineLayout, 0, 1,
                          &descriptor.sets[_mechanics.syncObjects.currentFrame],
                          0, nullptr);

  vkCmdDraw(commandBuffer, 36, _control.grid.numberOfGridPoints, 0, 0);

  vkCmdEndRenderPass(commandBuffer);

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }
}

void MemoryCommands::createBuffer(VkDeviceSize size,
                                  VkBufferUsageFlags usage,
                                  VkMemoryPropertyFlags properties,
                                  VkBuffer& buffer,
                                  VkDeviceMemory& bufferMemory) {
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  _log.console("{ ... }",
               "creating Buffer:", _log.getBufferUsageString(bufferInfo.usage));
  _log.console(_log.style.charLeader, bufferInfo.size, "bytes");

  if (vkCreateBuffer(_mechanics.mainDevice.logical, &bufferInfo, nullptr,
                     &buffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to create buffer!");
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(_mechanics.mainDevice.logical, buffer,
                                &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(_mechanics.mainDevice.logical, &allocInfo, nullptr,
                       &bufferMemory) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate buffer memory!");
  }

  vkBindBufferMemory(_mechanics.mainDevice.logical, buffer, bufferMemory, 0);
}

void MemoryCommands::copyBuffer(VkBuffer srcBuffer,
                                VkBuffer dstBuffer,
                                VkDeviceSize size) {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = command.pool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(_mechanics.mainDevice.logical, &allocInfo,
                           &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  VkBufferCopy copyRegion{};
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

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

  throw std::runtime_error("failed to find suitable memory type!");
}
