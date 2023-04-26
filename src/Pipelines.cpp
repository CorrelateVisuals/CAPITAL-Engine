#include <vulkan/vulkan.h>

#include <array>
#include <random>

#include "Capital_Engine.h"
#include "Debug.h"
#include "Pipelines.h"
#include "Vulkan_Mechanics.h"
#include "World.h"

Pipelines::Pipelines() : graphics{}, compute{} {
  LOG.console("{ P }", "constructing Pipelines");
}

Pipelines::~Pipelines() {
  LOG.console("{ P }", "destructing Pipelines");
}

void Pipelines::createDescriptorSetLayout() {
  LOG.console("  ....  ", "creating Descriptor Set Layout");
  constexpr int numBindings = 4;
  std::array<VkDescriptorSetLayoutBinding, numBindings> layoutBindings{};

  // Compute Shader input
  layoutBindings[0].binding = 0;
  layoutBindings[0].descriptorCount = 1;
  layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  layoutBindings[0].pImmutableSamplers = nullptr;
  layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

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

  // Vertex Shader input
  layoutBindings[3].binding = 3;
  layoutBindings[3].descriptorCount = 1;
  layoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  layoutBindings[3].pImmutableSamplers = nullptr;
  layoutBindings[3].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = numBindings;
  layoutInfo.pBindings = layoutBindings.data();

  if (vkCreateDescriptorSetLayout(MECHANICS.mainDevice.logical, &layoutInfo,
                                  nullptr, &MEM_COMMANDS.descriptorSetLayout) !=
      VK_SUCCESS) {
    throw std::runtime_error(
        "! ! ! ! ! ! ! failed to create compute descriptor set layout!");
  }
}

void Pipelines::createGraphicsPipeline() {
  LOG.console("{ P }", "creating Graphics Pipeline");
  auto vertShaderCode = readShaderFiles("shaders/vert.spv");
  auto fragShaderCode = readShaderFiles("shaders/frag.spv");

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

  auto bindingDescription = World::Cell::getBindingDescription();
  auto attributeDescriptions = World::Cell::getAttributeDescriptions();

  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.vertexAttributeDescriptionCount =
      static_cast<uint32_t>(attributeDescriptions.size());
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
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
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
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
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = &MEM_COMMANDS.descriptorSetLayout;

  if (vkCreatePipelineLayout(MECHANICS.mainDevice.logical, &pipelineLayoutInfo,
                             nullptr, &graphics.pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("! ! ! ! ! ! ! failed to create pipeline layout!");
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
  pipelineInfo.renderPass = RENDER_CONFIG.renderPass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  if (vkCreateGraphicsPipelines(MECHANICS.mainDevice.logical, VK_NULL_HANDLE, 1,
                                &pipelineInfo, nullptr,
                                &graphics.pipeline) != VK_SUCCESS) {
    throw std::runtime_error(
        "! ! ! ! ! ! ! failed to create graphics pipeline!");
  }

  vkDestroyShaderModule(MECHANICS.mainDevice.logical, fragShaderModule,
                        nullptr);
  vkDestroyShaderModule(MECHANICS.mainDevice.logical, vertShaderModule,
                        nullptr);
}

void Pipelines::createComputePipeline() {
  LOG.console("{ P }", "creating Compute Pipleline");
  auto computeShaderCode = readShaderFiles("shaders/comp.spv");

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
  pipelineLayoutInfo.pSetLayouts = &MEM_COMMANDS.descriptorSetLayout;

  if (vkCreatePipelineLayout(MECHANICS.mainDevice.logical, &pipelineLayoutInfo,
                             nullptr, &compute.pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error(
        "! ! ! ! ! ! ! failed to create compute pipeline layout!");
  }

  VkComputePipelineCreateInfo pipelineInfo{};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  pipelineInfo.layout = compute.pipelineLayout;
  pipelineInfo.stage = computeShaderStageInfo;

  if (vkCreateComputePipelines(MECHANICS.mainDevice.logical, VK_NULL_HANDLE, 1,
                               &pipelineInfo, nullptr,
                               &compute.pipeline) != VK_SUCCESS) {
    throw std::runtime_error(
        "! ! ! ! ! ! ! failed to create compute pipeline!");
  }

  vkDestroyShaderModule(MECHANICS.mainDevice.logical, computeShaderModule,
                        nullptr);
}

std::vector<char> Pipelines::readShaderFiles(const std::string& filename) {
  LOG.console("  ....  ", "reading Shader Files");

  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("! ! ! ! ! ! ! failed to open shader file :" +
                             filename);
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();

  return buffer;
}

VkShaderModule Pipelines::createShaderModule(const std::vector<char>& code) {
  LOG.console("  ....  ", "creating Shader Module");

  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = code.size();
  createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

  VkShaderModule shaderModule;
  if (vkCreateShaderModule(MECHANICS.mainDevice.logical, &createInfo, nullptr,
                           &shaderModule) != VK_SUCCESS) {
    throw std::runtime_error("! ! ! ! ! ! ! failed to create shader module!");
  }

  return shaderModule;
}

MemoryCommands::MemoryCommands()
    : commandPool{},
      commandBuffers{},
      computeCommandBuffers{},
      descriptorSetLayout{},
      descriptorPool{},
      computeDescriptorSets{} {
  LOG.console("{ 0 }", "constructing Memory Management");
}

MemoryCommands::~MemoryCommands() {
  LOG.console("{ 0 }", "destructing Memory Management");
}

void MemoryCommands::createCommandPool() {
  LOG.console("{ cmd }", "creating Command Pool");
  VulkanMechanics::QueueFamilyIndices queueFamilyIndices =
      MECHANICS.findQueueFamilies(MECHANICS.mainDevice.physical);

  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

  if (vkCreateCommandPool(MECHANICS.mainDevice.logical, &poolInfo, nullptr,
                          &commandPool) != VK_SUCCESS) {
    throw std::runtime_error("! ! ! ! ! ! ! failed to create command pool!");
  }
}

void MemoryCommands::createCommandBuffers() {
  LOG.console("  ....  ", "creating Command Buffers");
  commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

  if (vkAllocateCommandBuffers(MECHANICS.mainDevice.logical, &allocInfo,
                               commandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error(
        "! ! ! ! ! ! ! failed to allocate command buffers!");
  }
}
void MemoryCommands::createComputeCommandBuffers() {
  LOG.console("  ....  ", "creating Compute Command Buffers");

  computeCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = (uint32_t)computeCommandBuffers.size();

  if (vkAllocateCommandBuffers(MECHANICS.mainDevice.logical, &allocInfo,
                               computeCommandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error(
        "! ! ! ! ! ! ! failed to allocate compute command buffers!");
  }
}

void MemoryCommands::createShaderStorageBuffers() {
  LOG.console("  ....  ", "creating Shader Storage Buffers");

  // Initialize particles
  std::default_random_engine rndEngine((unsigned)time(nullptr));
  std::uniform_real_distribution<float> rndDist(0.0f, 1.0f);

  // Initial particle positions on a circle
  std::vector<World::Cell> particles(CELL_COUNT);
  for (auto& particle : particles) {
    float r = 0.25f * sqrt(rndDist(rndEngine));
    float theta = rndDist(rndEngine) * 2.0f * 3.14159265358979323846f;
    float x = r * cos(theta) * displayConfig.height / displayConfig.width;
    float y = r * sin(theta);
    particle.position = glm::vec2(x, y);
    particle.velocity = glm::normalize(glm::vec2(x, y)) * 0.00025f;
    particle.color = glm::vec4(rndDist(rndEngine), rndDist(rndEngine),
                               rndDist(rndEngine), 1.0f);
  }

  VkDeviceSize bufferSize = sizeof(World::Cell) * CELL_COUNT;

  // Create a staging buffer used to upload data to the gpu
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               stagingBuffer, stagingBufferMemory);

  void* data;
  vkMapMemory(MECHANICS.mainDevice.logical, stagingBufferMemory, 0, bufferSize,
              0, &data);
  memcpy(data, particles.data(), (size_t)bufferSize);
  vkUnmapMemory(MECHANICS.mainDevice.logical, stagingBufferMemory);

  shaderStorageBuffers.resize(MAX_FRAMES_IN_FLIGHT);
  shaderStorageBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

  // Copy initial particle data to all storage buffers
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    createBuffer(bufferSize,
                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                     VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, shaderStorageBuffers[i],
                 shaderStorageBuffersMemory[i]);
    copyBuffer(stagingBuffer, shaderStorageBuffers[i], bufferSize);
  }

  vkDestroyBuffer(MECHANICS.mainDevice.logical, stagingBuffer, nullptr);
  vkFreeMemory(MECHANICS.mainDevice.logical, stagingBufferMemory, nullptr);
}

void MemoryCommands::createUniformBuffers() {
  LOG.console("  ....  ", "creating Uniform Buffers");

  VkDeviceSize bufferSize = sizeof(UniformBufferObject);

  uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
  uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
  uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 uniformBuffers[i], uniformBuffersMemory[i]);

    vkMapMemory(MECHANICS.mainDevice.logical, uniformBuffersMemory[i], 0,
                bufferSize, 0, &uniformBuffersMapped[i]);
  }
}

void MemoryCommands::createDescriptorPool() {
  const int numPools = 2;
  LOG.console("  ....  ", "creating", numPools, "Descriptor Pools");

  std::array<VkDescriptorPoolSize, numPools> poolSizes{};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

  poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  poolSizes[1].descriptorCount =
      static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * numPools;

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = numPools;
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

  if (vkCreateDescriptorPool(MECHANICS.mainDevice.logical, &poolInfo, nullptr,
                             &descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("! ! ! ! ! ! ! failed to create descriptor pool!");
  }
}
void MemoryCommands::createComputeDescriptorSets() {
  LOG.console("  ....  ", "creating Compute Descriptor Sets");

  std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT,
                                             descriptorSetLayout);
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
  allocInfo.pSetLayouts = layouts.data();

  computeDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
  if (vkAllocateDescriptorSets(MECHANICS.mainDevice.logical, &allocInfo,
                               computeDescriptorSets.data()) != VK_SUCCESS) {
    throw std::runtime_error(
        "! ! ! ! ! ! ! failed to allocate descriptor sets!");
  }

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    VkDescriptorBufferInfo uniformBufferInfo{};
    uniformBufferInfo.buffer = uniformBuffers[i];
    uniformBufferInfo.offset = 0;
    uniformBufferInfo.range = sizeof(UniformBufferObject);

    std::array<VkWriteDescriptorSet, 3> descriptorWrites{};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = computeDescriptorSets[i];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &uniformBufferInfo;

    VkDescriptorBufferInfo storageBufferInfoLastFrame{};
    storageBufferInfoLastFrame.buffer =
        shaderStorageBuffers[(i - 1) % MAX_FRAMES_IN_FLIGHT];
    storageBufferInfoLastFrame.offset = 0;
    storageBufferInfoLastFrame.range = sizeof(World::Cell) * CELL_COUNT;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = computeDescriptorSets[i];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pBufferInfo = &storageBufferInfoLastFrame;

    VkDescriptorBufferInfo storageBufferInfoCurrentFrame{};
    storageBufferInfoCurrentFrame.buffer = shaderStorageBuffers[i];
    storageBufferInfoCurrentFrame.offset = 0;
    storageBufferInfoCurrentFrame.range = sizeof(World::Cell) * CELL_COUNT;

    descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[2].dstSet = computeDescriptorSets[i];
    descriptorWrites[2].dstBinding = 2;
    descriptorWrites[2].dstArrayElement = 0;
    descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrites[2].descriptorCount = 1;
    descriptorWrites[2].pBufferInfo = &storageBufferInfoCurrentFrame;

    vkUpdateDescriptorSets(MECHANICS.mainDevice.logical, 3,
                           descriptorWrites.data(), 0, nullptr);
  }
}

void MemoryCommands::updateUniformBuffer(uint32_t currentImage) {
  UniformBufferObject ubo{};
  ubo.deltaTime = lastFrameTime * 2.0f;

  memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void MemoryCommands::recordComputeCommandBuffer(VkCommandBuffer commandBuffer) {
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error(
        "! ! ! ! ! ! ! failed to begin recording compute command buffer!");
  }

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                    PIPELINES.compute.pipeline);

  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                          PIPELINES.compute.pipelineLayout, 0, 1,
                          &computeDescriptorSets[MECHANICS.currentFrame], 0,
                          nullptr);

  vkCmdDispatch(commandBuffer, CELL_COUNT / 256, 1, 1);

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error(
        "! ! ! ! ! ! ! failed to record compute command buffer!");
  }
}

void MemoryCommands::recordCommandBuffer(VkCommandBuffer commandBuffer,
                                         uint32_t imageIndex) {
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error(
        "! ! ! ! ! ! ! failed to begin recording command buffer!");
  }

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = RENDER_CONFIG.renderPass;
  renderPassInfo.framebuffer = MECHANICS.swapChainFramebuffers[imageIndex];
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = MECHANICS.swapChainExtent;

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
  clearValues[1].depthStencil = {1.0f, 0};

  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    PIPELINES.graphics.pipeline);

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)MECHANICS.swapChainExtent.width;
  viewport.height = (float)MECHANICS.swapChainExtent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = MECHANICS.swapChainExtent;
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1,
                         &shaderStorageBuffers[MECHANICS.currentFrame],
                         offsets);

  vkCmdDraw(commandBuffer, CELL_COUNT, 1, 0, 0);

  vkCmdEndRenderPass(commandBuffer);

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("! ! ! ! ! ! ! failed to record command buffer!");
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

  if (vkCreateBuffer(MECHANICS.mainDevice.logical, &bufferInfo, nullptr,
                     &buffer) != VK_SUCCESS) {
    throw std::runtime_error("! ! ! ! ! ! ! failed to create buffer!");
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(MECHANICS.mainDevice.logical, buffer,
                                &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(MECHANICS.mainDevice.logical, &allocInfo, nullptr,
                       &bufferMemory) != VK_SUCCESS) {
    throw std::runtime_error("! ! ! ! ! ! ! failed to allocate buffer memory!");
  }

  vkBindBufferMemory(MECHANICS.mainDevice.logical, buffer, bufferMemory, 0);
}

void MemoryCommands::copyBuffer(VkBuffer srcBuffer,
                                VkBuffer dstBuffer,
                                VkDeviceSize size) {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = commandPool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(MECHANICS.mainDevice.logical, &allocInfo,
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

  vkQueueSubmit(MECHANICS.queues.graphics, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(MECHANICS.queues.graphics);

  vkFreeCommandBuffers(MECHANICS.mainDevice.logical, commandPool, 1,
                       &commandBuffer);
}

uint32_t MemoryCommands::findMemoryType(uint32_t typeFilter,
                                        VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(MECHANICS.mainDevice.physical,
                                      &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags &
                                    properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error(
      "! ! ! ! ! ! ! failed to find suitable memory type!");
}
