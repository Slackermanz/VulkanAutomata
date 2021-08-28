## VulkanAutomata
Clone the VKAutoGLFW branch of VulkanAutomata.<br>
https://github.com/Slackermanz/VulkanAutomata/tree/VKAutoGLFW<br>
You could use this command to do it: `git clone --branch VKAutoGLFW https://github.com/Slackermanz/VulkanAutomata`

## GLFW
Download and install the GLFW development library.<br>
You need to download the 32-bit binaries so that it works with MinGW.<br>
https://www.glfw.org/download<br>

Also, find `glfw3.dll` and place it into the `app` folder.


## Vulkan
Download and install the Vulkan SDK.<br>
The one called something like: VulkanSDK-1.2.182.0-Installer.exe<br>
https://vulkan.lunarg.com/sdk/home

## MinGW
Download and install MinGW so that you can use the `g++` command to compile stuff.<br>
https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/installer/mingw-w64-install.exe/download<br>
You might need to add it to your path so that you can use commands like `g++`.<br>
For me, I added this to my path: `C:\MinGW\bin`

## Makefile
Make a `.ps1` file with this code. I called mine `make.ps1`.
```
glslc -O ./res/vert/vert_TriQuad.vert -o ./app/vert_TriQuad.spv
glslc -O ./res/frag/frag_automata0000.frag -o ./app/frag_automata0000.spv
g++ VulkanAutomataGLFW.cpp -fconcepts -L C:\glfw-3.3.4.bin.WIN32\lib-mingw -lglfw3dll -L C:\VulkanSDK\1.2.170.0\Lib32 -lvulkan-1 -I C:\VulkanSDK\1.2.170.0\Include -I C:\glfw-3.3.4.bin.WIN32\include -o ./app/RunVkAuto 
./app/RunVkAuto
```

## Includes
Now you need to edit `make.ps1` with the locations of your Vulkan and GLFW installs.<br>
For example, do you see where it says `C:\glfw-3.3.4.bin.WIN32\lib-mingw`? Change that to where your GLFW install is.<br>
Do that for all four locations.

## Go!
Run `make.js` in powershell with a command like this: `./make.ps1`
