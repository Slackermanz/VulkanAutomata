# VulkanAutomata
GPU-Accelerated Cellular Automata Render Engine using the Vulkan API

Developed on `Xubuntu 20.04`

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

Controls:

`(LSHIFT+)Left Mouse` : Place Cells

`(LSHIFT+)Right Mouse` :  (Left drag, right drag) Edit Scale value | (Shift) Remove Cells 

`(LSHIFT+)Middle Mouse Click` : Save Pattern | (Shift) Save Pattern and move to newest pattern on file

`(LSHIFT+)Forward Mouse Thumb or RIGHT ARROW` : Increment Saved Pattern Index | (Shift) Increment Saved Pattern Index by 100

`(LSHIFT+)Back Mouse Thumb or LEFT ARROW` : Decrement Saved Pattern Index | (Shift) Decrement Saved Pattern Index by 100

`R` : Randomise all pattern config values

`LSHIFT` : Reseed surface

`V` : Mutate one step from currently loaded pattern (Main mutation tool)

`C` : Mutate one step from currently displayed pattern

`X` : Erase surface

`Z` : Apply symmetrical seed

`Q` : Reload currently loaded pattern (erase unsaved mutations)

`1` : Default view mode

`6` : Toggle Linear Parameter Map

`Numpad ENTER` : begin recording frames

`Numpad .` : Make video from exported frames

`Numpad +` : Increase recording frame-skip

`Numpad -` : Decrease recording frame-skip

`SPACEBAR` : Toggle Pause

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


