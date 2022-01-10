cmake_minimum_required(VERSION 3.19)

#C++ 17 standard
set(CMAKE_CXX_STANDARD 17)

project(VulkanAutomata LANGUAGES CXX)

#source_files will contain all .cpp files under src and its subdirectories
file(GLOB_RECURSE source_files 
  ${CMAKE_SOURCE_DIR}/src/*.cpp
)

add_executable(vulkan_automata
  ${source_files}
  SlackAutomata.cpp
)

target_include_directories(
  vulkan_automata
  PUBLIC
    "src"
)

find_package(Vulkan REQUIRED)
target_link_libraries(vulkan_automata Vulkan::Vulkan)

