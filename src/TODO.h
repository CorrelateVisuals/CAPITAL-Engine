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
/// - Dear ImGui (for runtime settings, graph data and economic encyclopedia)
/// - Multisampling
/// - Renderpicking: Adding screenspace position to vertex ID, to click on
/// objects and return their IDs. A Vulkan attachment is needed for this
/// - Low Frequency Oscilator. That animates a value smoothly between a
/// range, based on a speed. Settings like lfo, ramp, step etc are usefull too
///
/// FEATURES: GEOMETRY
/// - Animating geometry between cells and implementing Touchdesigner fragment
/// shader proof of concept in the compute shader
/// - Terrain (needs new pipeline)
///		- Add landownership vinroy effect (camera that films cones that
///		  overlap, as texture on landscape)
///
///
/// </TO DO> ///////////////////////////////////////////////////////////////////
