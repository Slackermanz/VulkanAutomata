#!/usr/bin/env bash
set -euo pipefail

# crop_tile.sh - Crop a region from PAM frames and tile the crops into one PNG.
#
# The default paths are derived from this script's repository location:
#   input:  <repo>/out
#   output: <repo>/img/tile_output.png
#
# All behavior is controlled by flags. Change this script when improving tool
# capability, not when selecting a different crop, frame subset, or output.

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
REPO_ROOT="$(cd -- "${SCRIPT_DIR}/.." && pwd -P)"

usage() {
    cat <<'EOF'
Usage:
  ./tools/crop_tile.sh [OPTIONS]

Purpose:
  Crop the same rectangle from a sequence of PAM frames, then tile the crops
  left-to-right, top-to-bottom into a single PNG for temporal inspection.

Options:
  -i, --input DIR        Input directory containing frame files
                         Default: <repo>/out
  -o, --output FILE      Output PNG path
                         Default: <repo>/img/tile_output.png
  -p, --pattern GLOB     Frame filename glob within input dir
                         Default: IMG*.PAM

  -x, --crop-x N         Crop X offset in pixels
                         Default: 0
  -y, --crop-y N         Crop Y offset in pixels
                         Default: 0
  -w, --width N          Crop width in pixels
                         Default: 256
  -H, --height N         Crop height in pixels
                         Default: 256
  -S, --scale N          Integer nearest-neighbor scale after crop
                         Default: 1

  -c, --cols N           Tile columns
                         Default: auto, near-square and landscape-biased
  -r, --rows N           Tile rows
                         Default: auto from columns/frame count
  -s, --step N           Frame step interval
                         Default: 1
  -n, --max-frames N     Maximum frames to include
                         Default: all selected by step

  -k, --keep-temps       Keep intermediate cropped PNG files
  -q, --quiet            Suppress informational output; print output path only
  -h, --help             Show this help

Examples:
  # Crop 256x256 from top-left of all frames, auto tile
  ./tools/crop_tile.sh

  # Crop 128x128 from center-ish of 2048x1024 frames, tile 4x4
  ./tools/crop_tile.sh --crop-x 960 --crop-y 448 --width 128 --height 128 --cols 4 --rows 4

  # Every 3rd frame, at most 16 frames
  ./tools/crop_tile.sh --step 3 --max-frames 16 --output ./img/sampled_tile.png

  # 2x nearest-neighbor upscale after crop
  ./tools/crop_tile.sh --width 128 --height 128 --scale 2
EOF
    exit "${1:-0}"
}

die() {
    echo "ERROR: $*" >&2
    exit 1
}

info() {
    if [ "${QUIET}" = false ]; then
        echo "$@"
    fi
}

require_command() {
    command -v "$1" >/dev/null 2>&1 || die "Required command not found: $1"
}

require_value() {
    local flag="$1"
    local value="${2:-}"
    if [ -z "$value" ]; then
        die "${flag} requires a value"
    fi
}

is_uint() {
    [[ "$1" =~ ^[0-9]+$ ]]
}

validate_uint() {
    local name="$1"
    local value="$2"
    local min="$3"

    is_uint "$value" || die "${name} must be an unsigned integer: ${value}"
    if [ "$value" -lt "$min" ]; then
        die "${name} must be >= ${min}: ${value}"
    fi
}

abs_dir() {
    local dir="$1"
    [ -d "$dir" ] || die "Directory not found: $dir"
    cd -- "$dir" && pwd -P
}

select_auto_layout() {
    local frame_count="$1"
    local best_cols=1
    local best_rows="$frame_count"
    local best_diff="$frame_count"

    for ((cols=1; cols<=frame_count; cols++)); do
        local rows=$(( (frame_count + cols - 1) / cols ))
        local diff=$(( cols - rows ))
        if [ "$diff" -lt 0 ]; then
            diff=$(( -diff ))
        fi

        if [ "$cols" -ge "$rows" ] && { [ "$diff" -lt "$best_diff" ] || [ "$best_cols" -lt "$best_rows" ]; }; then
            best_cols="$cols"
            best_rows="$rows"
            best_diff="$diff"
        fi
    done

    TILE_COLS="$best_cols"
    TILE_ROWS="$best_rows"
}

# Defaults
INPUT_DIR="${REPO_ROOT}/out"
OUTPUT_FILE="${REPO_ROOT}/img/tile_output.png"
FRAME_PATTERN="IMG*.PAM"

CROP_X=0
CROP_Y=0
CROP_W=256
CROP_H=256
UPSCALE=1

TILE_COLS=""
TILE_ROWS=""
FRAME_STEP=1
MAX_FRAMES=""

KEEP_TEMPS=false
QUIET=false

while [ "$#" -gt 0 ]; do
    case "$1" in
        -h|--help)
            usage 0
            ;;
        -i|--input)
            require_value "$1" "${2:-}"
            INPUT_DIR="$2"
            shift 2
            ;;
        --input=*)
            INPUT_DIR="${1#*=}"
            shift
            ;;
        -o|--output)
            require_value "$1" "${2:-}"
            OUTPUT_FILE="$2"
            shift 2
            ;;
        --output=*)
            OUTPUT_FILE="${1#*=}"
            shift
            ;;
        -p|--pattern)
            require_value "$1" "${2:-}"
            FRAME_PATTERN="$2"
            shift 2
            ;;
        --pattern=*)
            FRAME_PATTERN="${1#*=}"
            shift
            ;;
        -x|--crop-x)
            require_value "$1" "${2:-}"
            CROP_X="$2"
            shift 2
            ;;
        --crop-x=*)
            CROP_X="${1#*=}"
            shift
            ;;
        -y|--crop-y)
            require_value "$1" "${2:-}"
            CROP_Y="$2"
            shift 2
            ;;
        --crop-y=*)
            CROP_Y="${1#*=}"
            shift
            ;;
        -w|--width)
            require_value "$1" "${2:-}"
            CROP_W="$2"
            shift 2
            ;;
        --width=*)
            CROP_W="${1#*=}"
            shift
            ;;
        -H|--height)
            require_value "$1" "${2:-}"
            CROP_H="$2"
            shift 2
            ;;
        --height=*)
            CROP_H="${1#*=}"
            shift
            ;;
        -S|--scale)
            require_value "$1" "${2:-}"
            UPSCALE="$2"
            shift 2
            ;;
        --scale=*)
            UPSCALE="${1#*=}"
            shift
            ;;
        -c|--cols)
            require_value "$1" "${2:-}"
            TILE_COLS="$2"
            shift 2
            ;;
        --cols=*)
            TILE_COLS="${1#*=}"
            shift
            ;;
        -r|--rows)
            require_value "$1" "${2:-}"
            TILE_ROWS="$2"
            shift 2
            ;;
        --rows=*)
            TILE_ROWS="${1#*=}"
            shift
            ;;
        -s|--step)
            require_value "$1" "${2:-}"
            FRAME_STEP="$2"
            shift 2
            ;;
        --step=*)
            FRAME_STEP="${1#*=}"
            shift
            ;;
        -n|--max-frames)
            require_value "$1" "${2:-}"
            MAX_FRAMES="$2"
            shift 2
            ;;
        --max-frames=*)
            MAX_FRAMES="${1#*=}"
            shift
            ;;
        -k|--keep-temps)
            KEEP_TEMPS=true
            shift
            ;;
        -q|--quiet)
            QUIET=true
            shift
            ;;
        *)
            die "Unknown option: $1"
            ;;
    esac
done

require_command ffmpeg
require_command ffprobe
require_command find
require_command sort

validate_uint "crop-x" "$CROP_X" 0
validate_uint "crop-y" "$CROP_Y" 0
validate_uint "width" "$CROP_W" 1
validate_uint "height" "$CROP_H" 1
validate_uint "scale" "$UPSCALE" 1
validate_uint "step" "$FRAME_STEP" 1

if [ -n "$MAX_FRAMES" ]; then
    validate_uint "max-frames" "$MAX_FRAMES" 1
fi
if [ -n "$TILE_COLS" ]; then
    validate_uint "cols" "$TILE_COLS" 1
fi
if [ -n "$TILE_ROWS" ]; then
    validate_uint "rows" "$TILE_ROWS" 1
fi
if [ -z "$FRAME_PATTERN" ]; then
    die "pattern must not be empty"
fi

INPUT_DIR="$(abs_dir "$INPUT_DIR")"

FRAMES=()
while IFS= read -r -d '' frame; do
    FRAMES+=("$frame")
done < <(find "$INPUT_DIR" -maxdepth 1 -type f -name "$FRAME_PATTERN" -print0 | sort -z -V)

if [ "${#FRAMES[@]}" -eq 0 ]; then
    die "No files matching ${FRAME_PATTERN} found in ${INPUT_DIR}"
fi

info "Found ${#FRAMES[@]} frames in ${INPUT_DIR}"

SELECTED=()
for ((idx=0; idx<${#FRAMES[@]}; idx+=FRAME_STEP)); do
    if [ -n "$MAX_FRAMES" ] && [ "${#SELECTED[@]}" -ge "$MAX_FRAMES" ]; then
        break
    fi
    SELECTED+=("${FRAMES[$idx]}")
done

FRAME_COUNT="${#SELECTED[@]}"
if [ "$FRAME_COUNT" -eq 0 ]; then
    die "No frames selected after applying step/max-frames"
fi

info "Selected ${FRAME_COUNT} frames (step=${FRAME_STEP})"

DIMENSIONS="$(ffprobe -v error -select_streams v:0 -show_entries stream=width,height -of csv=s=x:p=0 "${SELECTED[0]}")"
if [ -z "$DIMENSIONS" ]; then
    die "Could not detect dimensions from ${SELECTED[0]}"
fi

IFS=x read -r INPUT_W INPUT_H <<< "$DIMENSIONS"
validate_uint "input width" "$INPUT_W" 1
validate_uint "input height" "$INPUT_H" 1
info "Input dimensions: ${INPUT_W}x${INPUT_H}"

if [ $((CROP_X + CROP_W)) -gt "$INPUT_W" ] || [ $((CROP_Y + CROP_H)) -gt "$INPUT_H" ]; then
    die "Crop region (${CROP_X},${CROP_Y})+(${CROP_W}x${CROP_H}) exceeds input ${INPUT_W}x${INPUT_H}"
fi

if [ -z "$TILE_COLS" ] && [ -z "$TILE_ROWS" ]; then
    select_auto_layout "$FRAME_COUNT"
elif [ -z "$TILE_COLS" ]; then
    TILE_COLS=$(( (FRAME_COUNT + TILE_ROWS - 1) / TILE_ROWS ))
elif [ -z "$TILE_ROWS" ]; then
    TILE_ROWS=$(( (FRAME_COUNT + TILE_COLS - 1) / TILE_COLS ))
fi

validate_uint "cols" "$TILE_COLS" 1
validate_uint "rows" "$TILE_ROWS" 1

TILE_CAPACITY=$((TILE_COLS * TILE_ROWS))
if [ "$TILE_CAPACITY" -lt "$FRAME_COUNT" ]; then
    die "Tile layout ${TILE_COLS}x${TILE_ROWS} has capacity ${TILE_CAPACITY}, but ${FRAME_COUNT} frames were selected"
fi

SCALED_W=$((CROP_W * UPSCALE))
SCALED_H=$((CROP_H * UPSCALE))
TILE_W=$((SCALED_W * TILE_COLS))
TILE_H=$((SCALED_H * TILE_ROWS))

info "Crop region: ${CROP_W}x${CROP_H} at offset (${CROP_X},${CROP_Y})"
info "Crop output per frame: ${SCALED_W}x${SCALED_H}"
info "Tile layout: ${TILE_COLS}x${TILE_ROWS} (${FRAME_COUNT} frames, capacity ${TILE_CAPACITY})"
info "Tile output dimensions: ${TILE_W}x${TILE_H}"

OUTPUT_PARENT="$(dirname -- "$OUTPUT_FILE")"
OUTPUT_NAME="$(basename -- "$OUTPUT_FILE")"
mkdir -p -- "$OUTPUT_PARENT"
OUTPUT_PARENT="$(abs_dir "$OUTPUT_PARENT")"
OUTPUT_FILE="${OUTPUT_PARENT}/${OUTPUT_NAME}"

CROP_FILTER="crop=${CROP_W}:${CROP_H}:${CROP_X}:${CROP_Y}"
if [ "$UPSCALE" -gt 1 ]; then
    CROP_FILTER="${CROP_FILTER},scale=${SCALED_W}:${SCALED_H}:flags=neighbor"
fi

WORK_DIR="$(mktemp -d "${OUTPUT_PARENT}/.crop_tile_tmp.XXXXXX")"
cleanup() {
    if [ "$KEEP_TEMPS" = false ]; then
        rm -rf -- "$WORK_DIR"
    fi
}
trap cleanup EXIT

info "Cropping selected frames..."
for ((i=0; i<FRAME_COUNT; i++)); do
    ffmpeg -loglevel error -y \
        -i "${SELECTED[$i]}" \
        -vf "$CROP_FILTER" \
        "${WORK_DIR}/crop_$(printf '%04d' "$i").png"
done

info "Tiling selected crops..."
ffmpeg -loglevel error -y \
    -i "${WORK_DIR}/crop_%04d.png" \
    -filter_complex "tile=${TILE_COLS}x${TILE_ROWS}:nb_frames=${FRAME_COUNT}" \
    -frames:v 1 \
    "$OUTPUT_FILE"

OUTPUT_SIZE="$(stat -c%s "$OUTPUT_FILE" 2>/dev/null || stat -f%z "$OUTPUT_FILE" 2>/dev/null || echo "?")"

info ""
info "Output: $OUTPUT_FILE"
info "Output size: ${OUTPUT_SIZE} bytes"
if [ "$KEEP_TEMPS" = true ]; then
    info "Intermediate crops: $WORK_DIR"
fi

echo "$OUTPUT_FILE"
