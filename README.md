# CAPITAL-engine
CAPITAL engine is a cross platform Vulkan engine for simulations and algorithms that benefit from parallel computing.

Based on the tutorial series by Sascha Willems.
https://vulkan-tutorial.com/Introduction

External Libraries
	// C++ Additional Include Directories
	$(SolutionDir)..\Libraries\glfw-3.3.8\include
	$(SolutionDir)..\Libraries\glm
	C:\VulkanSDK\1.3.224.1\Include

	//Linker Additional Libraries Directories
	C:\VulkanSDK\1.3.224.1\Lib
	$(SolutionDir)..\Libraries\glfw-3.3.8\lib-vc2022

	//Linker Additional Dependencies
	vulkan-1.lib
	glfw3.lib