# VulkanAutomata
GPU-Accelerated Cellular Automata Render Engine using the Vulkan API

Developed on `Xubuntu 20.04`

---

The Cellular Automata 'rules' are coded as Fragment Shaders in `./res/frag/` 
 
Shaders are compiled from `./res/vert/` and  `./res/frag/` using the program `glslc`

`glslc` is found in the Vulkan SDK at `/x86_64/bin/glslc`

The Vulkan SDK can be found at: https://vulkan.lunarg.com/sdk/home

---

Commands should be called from the location `./VulkanAutomata`

Build & Run:

`script_buildrun.sh`

---

Contacts & Networking:

  https://twitter.com/slackermanz

  https://www.reddit.com/user/slackermanz/

  https://www.youtube.com/channel/UCmoNsNuM0M9VsIXfm2cHPiA/videos

  https://www.shadertoy.com/user/SlackermanzCA

  https://discord.gg/BCuYCEn

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
