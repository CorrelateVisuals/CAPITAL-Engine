#include "World.h"

World::World()
{
}

World::~World()
{
}

VkVertexInputBindingDescription World::Cell::getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Cell);
    bindingDescription.inputRate =
        VK_VERTEX_INPUT_RATE_VERTEX;  // VK_VERTEX_INPUT_RATE_INSTANCE

    return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription>
World::Cell::getAttributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

    attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Cell, pos) });
    attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Cell, color) });
    attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Cell, normals) });
    attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Cell, uv) });


    return attributeDescriptions;
}
