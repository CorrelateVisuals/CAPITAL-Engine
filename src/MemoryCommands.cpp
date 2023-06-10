#include "MemoryCommands.h"
#include "CapitalEngine.h"
#include "Debug.h"
#include "Pipelines.h"

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
