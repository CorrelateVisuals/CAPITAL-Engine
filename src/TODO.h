#pragma once
/// <TO DO> ///////////////////////////////////////////////////////////////////
///
/// [ FIND BUGS ]
/// - Check if functions work how they are supposed to work
///
/// [ BUGS ]
/// - Adding 'long long' data type to shader, with enabling the right extension
/// on the Vulkan side
/// - Remove the vertex, index and normal data from vertex shader to c++
/// - Optimization and bug fixes
/// - Revise the _log.console() function (with the use of overload functions)
/// - Format runtime errors to right style format
/// - Adding CPU access to cell Shader data through shaderStorage buffer
///
/// [ FEATURES: ADDITIONS ]
/// - Dear ImGui (for runtime settings, graph data and economic encyclopedia)
/// - Multisampling
/// - Renderpicking: Adding screenspace position to vertex ID, to click on
/// objects and return their IDs. A Vulkan attachment is needed for this
/// - Ray-tracing
///
/// [ FEATURES: GEOMETRY ]
/// - Animating geometry between cells and implementing Touchdesigner fragment
/// shader proof of concept in the compute shader
/// - Terrain (needs new pipeline)
///		- Add landownership vinroy effect (camera that films cones that
///		  overlap, as texture on landscape)
///
/// </TO DO> ///////////////////////////////////////////////////////////////////
