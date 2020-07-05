# VulkanAutomata
GPU Renderer for Cellular Automata using the Vulkan API

---

The Cellular Automata 'rules' are coded as Fragment Shaders
 
Shaders are compiled from `./res/vert` and  `./res/frag` using the program `glslc`

`glslc` is found in the Vulkan SDK at `/x86_64/bin/glslc`

The Vulkan SDK can be found at: https://vulkan.lunarg.com/sdk/home

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

Vulkan SDK: `1.2.141.2`

g++: `(Ubuntu 9.3.0-10ubuntu2) 9.3.0`

Lines added to `/home/<USER>/.profile`:

    export VULKAN_SDK=/home/<USER>/1.2.141.2/x86_64
    export PATH="$VULKAN_SDK/bin:$PATH"
    export LD_LIBRARY_PATH="$VULKAN_SDK/lib:${LD_LIBRARY_PATH:-}"
    export VK_LAYER_SETTINGS_PATH="$VULKAN_SDK/config"
    export VK_LAYER_PATH="$VULKAN_SDK/etc/vulkan/explicit_layer.d"
    
---

`vulkaninfo`:

` Vulkan Instance Version: 1.2.131`
    
`VK_LAYER_KHRONOS_validation Vulkan version 1.2.141, layer version 1`
    
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

