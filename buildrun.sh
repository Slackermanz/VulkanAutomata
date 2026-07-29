#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
cd "${SCRIPT_DIR}"

CXX="${CXX:-g++}"
GLSLC="${GLSLC:-glslc}"
GLFW_PKG="${GLFW_PKG:-glfw3}"

die() {
    echo "ERROR: $*" >&2
    exit 1
}

command -v "${CXX}" >/dev/null 2>&1 || die "Required C++ compiler not found: ${CXX}"
command -v "${GLSLC}" >/dev/null 2>&1 || die "Required shader compiler not found: ${GLSLC}"
command -v pkg-config >/dev/null 2>&1 || die "Required command not found: pkg-config"
pkg-config --exists "${GLFW_PKG}" ||
    die "pkg-config could not resolve ${GLFW_PKG}. Install the GLFW development package so ${GLFW_PKG}.pc is available."

GLFW_CFLAGS_TEXT="$(pkg-config --cflags "${GLFW_PKG}")"
GLFW_LIBS_TEXT="$(pkg-config --libs "${GLFW_PKG}")"
read -r -a GLFW_CFLAGS <<< "${GLFW_CFLAGS_TEXT}"
read -r -a GLFW_LIBS <<< "${GLFW_LIBS_TEXT}"

printf '#include <GLFW/glfw3.h>\n' |
    "${CXX}" -std=c++17 -fconcepts "${GLFW_CFLAGS[@]}" -x c++ -fsyntax-only - >/dev/null ||
    die "Compiler cannot resolve GLFW/glfw3.h with pkg-config supplied flags."

printf '#include <vulkan/vulkan.h>\n' |
    "${CXX}" -std=c++17 -fconcepts -x c++ -fsyntax-only - >/dev/null ||
    die "Compiler cannot resolve vulkan/vulkan.h. Install the Vulkan development headers."

PROBE_BIN="$(mktemp "${SCRIPT_DIR}/app/.buildrun-link.XXXXXX")"
trap 'rm -f -- "${PROBE_BIN}"' EXIT
printf '#include <GLFW/glfw3.h>\n#include <vulkan/vulkan.h>\nint main() { (void)&glfwInit; (void)&vkCreateInstance; return 0; }\n' |
    "${CXX}" -std=c++17 -fconcepts "${GLFW_CFLAGS[@]}" -x c++ - \
        "${GLFW_LIBS[@]}" -lvulkan -o "${PROBE_BIN}" >/dev/null ||
    die "Link probe failed for GLFW and Vulkan. Verify the GLFW development files and Vulkan loader library are installed."

echo ""
echo "  PREFLIGHT OK"

rm -f \
    ./app/vert_TriQuad.spv \
    ./app/frag_automata0000.spv \
    ./app/RunVulkanAutomataGLFW

echo ""
echo "  ${GLSLC} -O ./res/vert/vert_TriQuad.vert -o ./app/vert_TriQuad.spv"
"${GLSLC}" -O ./res/vert/vert_TriQuad.vert -o ./app/vert_TriQuad.spv

echo ""
echo "  ${GLSLC} -O ./res/frag/frag_automata0000.frag -o ./app/frag_automata0000.spv"
"${GLSLC}" -O ./res/frag/frag_automata0000.frag -o ./app/frag_automata0000.spv

SOURCES=(
    lib/*.cpp
    vkmodules/Utils/*.cpp
    vkmodules/CellularAutomata/*.cpp
    vkmodules/UI/*.cpp
    vkmodules/Input/*.cpp
    vkmodules/VulkanFoundation/*.cpp
    vkmodules/CommandBuffer/*.cpp
    vkmodules/Resources/*.cpp
    vkmodules/Rendering/*.cpp
    vkmodules/Platform/*.cpp
    vkmodules/Core/*.cpp
    vkmodules/Export/*.cpp
    VulkanAutomataGLFW.cpp
)

echo ""
echo "  ${CXX} -g -std=c++17 ... -fconcepts \$(pkg-config --cflags --libs ${GLFW_PKG}) -lvulkan -o ./app/RunVulkanAutomataGLFW"
"${CXX}" -g -std=c++17 \
    "${SOURCES[@]}" \
    -fconcepts \
    "${GLFW_CFLAGS[@]}" \
    "${GLFW_LIBS[@]}" \
    -lvulkan \
    -o ./app/RunVulkanAutomataGLFW

echo ""
echo "  BEGIN ./app/RunVulkanAutomataGLFW"
echo ""
./app/RunVulkanAutomataGLFW
echo ""
echo "  END   ./app/RunVulkanAutomataGLFW"
echo ""
