# VulkanAutomata
GPU-Accelerated Cellular Automata Render Engine using the Vulkan API

Developed on `Xubuntu 20.04`

---

Demo video: https://www.youtube.com/watch?v=MSINHosdRjU

---

The Vulkan SDK can be found at: https://vulkan.lunarg.com/sdk/home

For `Xubuntu 20.04`:

`wget -qO - https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo apt-key add -`

`sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-focal.list https://packages.lunarg.com/vulkan/lunarg-vulkan-focal.list`

`sudo apt update`

`sudo apt install vulkan-sdk`

---

The Cellular Automata 'rules' are coded as Fragment Shaders in `./res/frag/` 
 
Shaders are compiled from `./res/vert/` and  `./res/frag/` using the program `glslc` from the Vulkan SDK

---

Scripts should be called from the location `./VulkanAutomata`

Build & Run:

`script_buildrun.sh`

or

`buildrun.sh`

or

`glslc -O ./res/vert/vert_TriQuad.vert -o ./app/vert_TriQuad.spv`

`glslc -O ./res/frag/frag_automata0000.frag -o ./app/frag_automata0000.spv`

`g++ VulkanAutomata.cpp -fconcepts -lvulkan -o ./app/RunVkAuto`

`./app/RunVkAuto`

---

Contacts & Networking:

 - Website: https://slackermanz.com

 - Discord: Slackermanz#3405

 - Github: https://github.com/Slackermanz

 - Twitter: https://twitter.com/slackermanz

 - YouTube: https://www.youtube.com/c/slackermanz

 - Shadertoy: https://www.shadertoy.com/user/SlackermanzCA

 - Reddit: https://old.reddit.com/user/slackermanz

---

Communities:

 - Emergence Discord: https://discord.com/invite/J3phjtD

 - ConwayLifeLounge Discord: https://discord.gg/BCuYCEn

 - Reddit: https://old.reddit.com/r/cellular_automata


