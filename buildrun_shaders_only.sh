#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
REPO_ROOT="${SCRIPT_DIR}"

cd "${REPO_ROOT}"

FRAG_SRC="${REPO_ROOT}/res/frag/frag_automata0000.frag"
FRAG_OUT="${REPO_ROOT}/app/frag_automata0000.spv"
BIN_OUT="${REPO_ROOT}/app/RunVulkanAutomataGLFW"
GLSLC="${GLSLC:-glslc}"

info() {
    echo "$@"
}

die() {
    echo "ERROR: $*" >&2
    exit 1
}

require_command() {
    command -v "$1" >/dev/null 2>&1 || die "Required command not found: $1"
}

main() {
    require_command "${GLSLC}"

    rm -f "${FRAG_OUT}"

    if command -v clear >/dev/null 2>&1 && [ -t 1 ]; then
        clear
    fi

    info ""
    info "  ${GLSLC} -O ./res/frag/frag_automata0000.frag -o ./app/frag_automata0000.spv"
    "${GLSLC}" -O "${FRAG_SRC}" -o "${FRAG_OUT}"

    if [ ! -x "${BIN_OUT}" ]; then
        die "Application binary not found at ./app/RunVulkanAutomataGLFW. Run ./buildrun.sh first."
    fi

    info ""
    info "  BEGIN ./app/RunVulkanAutomataGLFW"
    info ""
    "${BIN_OUT}"
    info ""
    info "  END   ./app/RunVulkanAutomataGLFW"
    info ""
}

main "$@"
