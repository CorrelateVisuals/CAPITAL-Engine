#pragma once
/// <TO DO> ///////////////////////////////////////////////////////////////////
///
/// FIXES
/// - Adding 'long long' data type to shader, with enabling the right extension
/// on the Vulkan side
/// - Correct the scaling of the compute row in relation to the grid size.
/// Currently sqrt() of one of the grid dimensions is used to do so and manually
/// this value has to be set in the compute shader
/// - Remove the vertex, index and normal data from vertex shader to c++
///
/// FEATURES: ADDITIONS
/// - Add Dear ImGui
/// - Add multisampling
/// - Adding screenspace position to vertex ID, to click on objects and return
/// their IDs (renderpicking). A Vulkan attachment is needed for this.
///
/// FEATURES: GEOMETRY
/// - Moving geometry between cells
/// - Add base terrain (landscape grid with height)
///		- Add landownership vinroy effect (camera that films cones that
///		  overlap, as texture on landscape)
///
///
/// </TO DO> ///////////////////////////////////////////////////////////////////
