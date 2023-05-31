#pragma once
/// <TO DO> ///////////////////////////////////////////////////////////////////
///
/// [ FIND BUGS ]
/// - Check if functions work as they are supposed to work
///
/// [ BUGS ]
/// - Adding mirror effect for cells outside of the grid to be added to the
/// other end of the grid
/// - Adding 'long long' data type to shader, with enabling the right extension
/// on the Vulkan side
///
/// [ FEATURES: ADDITIONS ]
/// - Add camera movements, arcball or common
/// https://github.com/Twinklebear/arcball-cpp
/// - Dear ImGui (for runtime settings, graph data and economic encyclopedia)
/// https://github.com/ocornut/imgui
/// - Multisampling
/// https://vulkan-tutorial.com/Multisampling
/// - Adding push constants
/// - Renderpicking/colorpicking: Adding screenspace position to vertex ID, to
/// click on objects and return their IDs.
/// https://snoozetime.github.io/2019/05/02/object-picking.html
/// - Add vertex and index buffers; remove the vertex, index and normal data
/// from vertex shader
/// https://vulkan-tutorial.com/Vertex_buffers/Vertex_buffer_creation
/// - Shader to SPIRV runtime conversion
///
/// [ FEATURES: GEOMETRY ]
/// - Animating geometry between cells
/// - Terrain (needs new pipeline?)
///		- Add landownership vinroy effect (camera that films cones that
///		  overlap, as texture on landscape)
///
/// </TO DO> ///////////////////////////////////////////////////////////////////
