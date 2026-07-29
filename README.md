# VulkanAutomata
GPU-Accelerated Cellular Automata Render Engine using the Vulkan API

Originally developed on `Xubuntu 20.04`.

This repository is currently built through the shell entrypoints in the repo root, not through CMake or Meson. The Linux build path assumes system-installed compiler and graphics development packages rather than a bundled project toolchain.

---

[![Alt text](https://img.youtube.com/vi/MSINHosdRjU/0.jpg)](https://www.youtube.com/watch?v=MSINHosdRjU)

Demo video: https://www.youtube.com/watch?v=MSINHosdRjU

---

## Build prerequisites

The current Linux build path requires:

- a C++17 compiler with concepts support
- `glslc`
- GLFW development headers and libraries
- Vulkan headers and loader libraries
- `pkg-config` so the build script can discover the active GLFW flags
- optional: Vulkan validation layers for runtime diagnostics

On Debian-like hosts, the relevant package names are typically:

- `g++`
- `pkg-config`
- `libglfw3-dev`
- `libvulkan-dev`
- `glslc`
- optional: `vulkan-validationlayers`

The historical LunarG SDK path can still be used if preferred, but it is not the only viable Linux setup. The repo build scripts now expect the active host environment to expose the necessary development surfaces directly.

---

## Build and run

Scripts should be called from the repository root `./VulkanAutomata`.

Primary entrypoints:

- `./buildrun.sh` — preflight the host toolchain, compile shaders, compile the application, then run it
- `./buildrun_shaders_only.sh` — rebuild the fragment shader and run the existing application binary
- `./script_buildrun.sh` — run `buildrun.sh` and capture the terminal transcript into `./log/`

Manual flow:

- compile `./res/vert/vert_TriQuad.vert` into `./app/vert_TriQuad.spv`
- compile `./res/frag/frag_automata0000.frag` into `./app/frag_automata0000.spv`
- compile the C++ application with GLFW flags resolved from the active system and link against Vulkan
- run `./app/RunVulkanAutomataGLFW`

If the host is missing required development packages, `buildrun.sh` now fails before the full compile and reports which surface is unavailable.

---

The Cellular Automata 'rules' are coded as Fragment Shaders in `./res/frag/`.

Shaders are compiled from `./res/vert/` and `./res/frag/` using `glslc`.

Further design notes:

- `docs/PAIR_SELECTIVE_MNCA.md` explains the active pair-selective perceptron MNCA shader architecture in approachable implementation terms.
- `docs/STATE_QUANTIZATION_AND_SYMMETRY.md` explains how the shaders use quantization boundaries to prevent hidden floating-point drift and preserve symmetry.

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
