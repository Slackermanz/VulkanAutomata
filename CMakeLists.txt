cmake_minimum_required(VERSION 3.16)

#C++ 17 standard
set(CMAKE_CXX_STANDARD 17)

project(VulkanAutomata LANGUAGES CXX)

#source_files will contain all .cpp files under src and its subdirectories
file(GLOB_RECURSE source_files 
  ${CMAKE_SOURCE_DIR}/source/*.cpp
)

add_executable(vulkan_automata
  ${source_files}
  SlackAutomata.cpp
)

target_include_directories(
  vulkan_automata
  PUBLIC
    "include"
)

find_package(Vulkan REQUIRED)
target_link_libraries(vulkan_automata Vulkan::Vulkan)


#Shaders

include(shaders.cmake)

set(shaders-dir ${CMAKE_SOURCE_DIR}/resources/shaders)

# Find all shaders.
file(GLOB_RECURSE vertex-shaders ${shaders-dir}/*.vert)
file(GLOB_RECURSE fragment-shaders ${shaders-dir}/*.frag)
file(GLOB_RECURSE compute-shaders ${shaders-dir}/*.comp)

# Add them to the build.
foreach(vertex-shader ${vertex-shaders})
	get_filename_component(p ${vertex-shader} NAME)
	add_shader(vulkan_automata ${p})
endforeach(vertex-shader)

foreach(fragment-shader ${fragment-shaders})
	get_filename_component(p ${fragment-shader} NAME)
	add_shader(vulkan_automata ${p})
endforeach(fragment-shader)

foreach(compute-shader ${compute-shaders})
	get_filename_component(p ${compute-shader} NAME)
	add_shader(vulkan_automata ${p})
endforeach(compute-shader)