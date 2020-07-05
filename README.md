# VulkanAutomata
GPU Renderer for Cellular Automata using the Vulkan API

---

The Cellular Automata 'rules' are coded as Fragment Shaders in `./res/frag` 
 
Shaders are compiled from `./res/vert` and  `./res/frag` using the program `glslc`

`glslc` is found in the Vulkan SDK at `/x86_64/bin/glslc`

The Vulkan SDK can be found at: https://vulkan.lunarg.com/sdk/home

---

Dependencies:

`https://github.com/KhronosGroup/Vulkan-Headers.git`
`https://github.com/KhronosGroup/Vulkan-Loader.git`
`https://github.com/KhronosGroup/Vulkan-Tools.git`
`https://github.com/KhronosGroup/SPIRV-Headers.git`
`https://github.com/KhronosGroup/SPIRV-Tools.git`
`https://github.com/KhronosGroup/Vulkan-ValidationLayers.git`

---

Commands should be called from the location `./VkAutomata`:

Build:

`./glslc ./res/vert/vert_TriQuad.vert -o ./app/vert_TriQuad.spv`

`./glslc ./res/frag/frag_automata0000.frag -o ./app/frag_automata0000.spv`

`./glslc ./res/frag/frag_init.frag -o ./app/frag_init.spv`

`g++ VulkanAutomata.cpp -fconcepts -lX11 -lvulkan -o ./app/RunVkAuto`

Run:

`./app/RunVkAuto`

---

Specific build versions used in development:

Operating System: `Xubuntu 20.04`

g++: `(Ubuntu 9.3.0-10ubuntu2) 9.3.0`

---

`vulkaninfo`:

`Vulkan Instance Version: 1.2.146`

`VK_LAYER_KHRONOS_validation (Khronos Validation Layer) Vulkan version 1.2.146`
    
    GPU Info:
      apiVersion         = 4198519 (1.1.119)
      driverVersion      = 1847132160 (0x6e190000)
      deviceType         = PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
      deviceName         = GeForce GTX 1070
      driverID           = DRIVER_ID_NVIDIA_PROPRIETARY
      driverName         = NVIDIA
      driverInfo         = 440.100
      conformanceVersion = 1.1.6.0

---

If you wish to contact me, send a message to https://old.reddit.com/user/slackermanz

