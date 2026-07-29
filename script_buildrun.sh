#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
LOG_DIR="${SCRIPT_DIR}/log"
LOG_FILE="${LOG_DIR}/AppLog$(date +%s).log"

command -v script >/dev/null 2>&1 || {
    echo "ERROR: Required command not found: script" >&2
    exit 1
}

mkdir -p "${LOG_DIR}"

build_status=0
script -q -e -c "${SCRIPT_DIR}/buildrun.sh" "${LOG_FILE}" || build_status=$?

echo ""
echo "    file://${LOG_FILE}"
echo ""

exit "${build_status}"
