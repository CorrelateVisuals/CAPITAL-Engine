#include "World.h"
#include "CAPITAL_Engine.h"

World::World() {
  _log.console("{ (:) }", "constructing World");
}

World::~World() {}

// VkVertexInputBindingDescription World::Cell::getBindingDescription() {
//   VkVertexInputBindingDescription bindingDescription{};
//   bindingDescription.binding = 0;
//   bindingDescription.stride = sizeof(Cell);
//   bindingDescription.inputRate =
//       VK_VERTEX_INPUT_RATE_VERTEX;  // VK_VERTEX_INPUT_RATE_INSTANCE
//
//   return bindingDescription;
// }
//
// static std::array<VkVertexInputAttributeDescription, 2>
// getAttributeDescriptions() {
//   std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
//
//   attributeDescriptions[0].binding = 0;
//   attributeDescriptions[0].location = 0;
//   attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
//   attributeDescriptions[0].offset = offsetof(World::Cell, position);
//
//   attributeDescriptions[1].binding = 0;
//   attributeDescriptions[1].location = 1;
//   attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
//   attributeDescriptions[1].offset = offsetof(World::Cell, color);
//
//   return attributeDescriptions;
// }
