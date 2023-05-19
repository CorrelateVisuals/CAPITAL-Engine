# CAPITAL-engine

CAPITAL engine is a cross platform Vulkan engine for simulations and algorithms that benefit from parallel computing.

## Windows development

External Libraries: [Vulkan](https://vulkan-tutorial.com/Development_environment)

```text
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
```

Based on the tutorial series by *Sascha Willems*: [Vulkan tutorial](https://vulkan-tutorial.com/Introduction).

## Linux Development

### Build management using CMake

Go to **build** sub-directory:

```bash
  cmake ..
  make -j
```

The excutable **CapitalEngine** is compiled in the **bin** sub-directory.

Compile the *.spv* files, go to the **shaders** sub-directory:

```bash
./compile_shaders.sh
```

### Executing

Go to the project root directory **CAPITAL-Engine**:

```bash
./bin/CapitalEngine
```
