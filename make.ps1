glslc -O ./res/vert/vert_TriQuad.vert -o ./app/vert_TriQuad.spv
glslc -O ./res/frag/frag_automata0000.frag -o ./app/frag_automata0000.spv
# g++ -std=c++17 lib/*.cpp VulkanAutomataGLFW.cpp -fconcepts -L C:\glfw-3.3.4.bin.WIN32\lib-mingw -lglfw3dll -L C:\VulkanSDK\1.2.170.0\Lib32 -lvulkan-1 -I C:\VulkanSDK\1.2.170.0\Include -I C:\glfw-3.3.4.bin.WIN32\include -o ./app/RunVkAuto 
# ./app/RunVKAuto
