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
Download and install MinGW from WinLibs, so that you can use the `g++` command to compile stuff.<br>
https://winlibs.com/<br>
Get the latest Win32 release version.<br>
Extract it somewhere and then add the `bin` folder to your path.<br>
For me, I had to my path: `C:\mingw32\bin`<br>

## Includes
Now you need to edit `make.ps1` (in this folder) with the locations of your Vulkan and GLFW installs.<br>
For example, do you see where it says `C:\glfw-3.3.4.bin.WIN32\lib-mingw`? Change that to where your GLFW install is.<br>
Do that for all four locations.

## Go!
Run `make.ps1` in powershell with a command like this: `./make.ps1`
