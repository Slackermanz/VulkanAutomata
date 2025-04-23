#!/bin/bash
starttime=$(date +%s)

# Ensure output directories exist
mkdir -p vid gif img

# Configuration for generated media
GIF_TARGET_DURATION=15  # seconds
GIF_TARGET_FPS=30       # frames per second
BSKY_TARGET_DURATION=20 # seconds
BSKY_TARGET_FPS=24      # frames per second
INPUT_ASSUMED_FPS=60    # Assumed input framerate for ffmpeg -framerate flag
BSKY_TARGET_W=2048 # Target width for primary Bluesky path scaling
BSKY_QUALITY_CRF=14 # CRF for Bluesky paths (lower is better quality, larger file)

# --- Detect Input Dimensions using ffprobe ---
first_frame="out/IMG0.PAM"
if [ ! -f "$first_frame" ]; then
    echo "ERROR: First frame '$first_frame' not found. Cannot determine input dimensions. Exiting."
    exit 1
fi
dimensions=$(ffprobe -v error -select_streams v:0 -show_entries stream=width,height -of csv=s=x:p=0 "$first_frame")
if [ -z "$dimensions" ]; then
    echo "ERROR: ffprobe failed to get dimensions from '$first_frame'. Ensure ffprobe is installed and the file is valid. Exiting."
    exit 1
fi
IFS=x read -r INPUT_W INPUT_H <<< "$dimensions"
echo "INFO: Detected input dimensions: ${INPUT_W}x${INPUT_H}"

# --- Bluesky Scaling Configuration (Primary Path) ---
# Calculate largest integer scaling factor that doesn't exceed target width
scaling_factor=$(awk -v w="$INPUT_W" -v tw="$BSKY_TARGET_W" 'BEGIN { factor = int(tw/w); if (factor < 1) factor = 1; printf "%d", factor }')
echo "INFO: Using primary scaling factor ${scaling_factor}x for Bluesky (Input W: ${INPUT_W}, Target W: ~${BSKY_TARGET_W})"

# Get filename for output sequence
fn=$(basename -s .mp4 vid/VKAutomata[0-9][0-9][0-9][0-9].mp4 2>/dev/null || echo "VKAutomata0000") # Handle case where no previous files exist
if [[ "$fn" == "VKAutomata0000" ]]; then
    idn=0
else
    c0=${#fn}
    let c0=c0-3
    c1=${#fn}
    idn=$(echo $fn | cut -c$c0-$c1)
    idn=$(( 10#$idn )) # Force base 10 interpretation
fi
let idn=idn+1
let pad=4-${#idn}
sf="0000"
sf=$(echo $sf | cut -c1-$pad)
sf=$sf$idn

echo "INFO: Generating output sequence number: $sf"

# Set inital "Thumbnail" frame, create seperate slides from potential thumnail frames
th="out/IMG0.PAM"
if test -f $th; then
    cp $th "out/SCR_INITAL.PAM"
fi
th="out/IMG1.PAM"
if test -f $th; then
    cp $th "out/IMG0.PAM"
    cp $th "out/SCR0.PAM"
fi
th="out/IMG10.PAM"
if test -f $th; then
    cp $th "out/SCR1.PAM"
fi
th="out/IMG60.PAM"
if test -f $th; then
    cp $th "out/SCR2.PAM"
fi
th="out/IMG180.PAM"
if test -f $th; then
    cp $th "out/SCR3.PAM"
fi
th="out/IMG600.PAM"
if test -f $th; then
    cp $th "out/SCR4.PAM"
fi
th="out/IMG2400.PAM"
if test -f $th; then
    cp $th "out/SCR5.PAM"
fi
th="out/IMG6400.PAM"
if test -f $th; then
    cp $th "out/SCR6.PAM"
fi
th="out/IMG18000.PAM"
if test -f $th; then
    cp $th "out/SCR7.PAM"
fi

echo "INFO: Generating screenshot PNGs..."
ffmpeg -loglevel error -y -i out/SCR%00d.PAM "img/VKAutomata${sf}_SCR%00d.png"

# Count total input frames
total_frames=$(ls ./out/IMG*.PAM 2>/dev/null | wc -l)
if [ $total_frames -eq 0 ]; then
    echo "ERROR: No input frames found in ./out/IMG*.PAM. Exiting."
    exit 1
fi
echo "INFO: Found $total_frames input frames."

# Calculate frame skip intervals
gif_target_total_frames=$((${GIF_TARGET_DURATION} * ${GIF_TARGET_FPS}))
gif_skip_interval=$(( (${total_frames} + ${gif_target_total_frames} - 1) / ${gif_target_total_frames} )) # Ceiling division
if [ ${gif_skip_interval} -lt 1 ]; then gif_skip_interval=1; fi
echo "INFO: GIF skip interval: ${gif_skip_interval} (Target: ${GIF_TARGET_DURATION}s @ ${GIF_TARGET_FPS}fps)"

bsky_target_total_frames=$((${BSKY_TARGET_DURATION} * ${BSKY_TARGET_FPS}))
bsky_skip_interval=$(( (${total_frames} + ${bsky_target_total_frames} - 1) / ${bsky_target_total_frames} )) # Ceiling division
if [ ${bsky_skip_interval} -lt 1 ]; then bsky_skip_interval=1; fi
echo "INFO: Bluesky skip interval: ${bsky_skip_interval} (Target: ${BSKY_TARGET_DURATION}s @ ${BSKY_TARGET_FPS}fps)"


# Video output filepaths/filenames (Original)
v0="vid/VKAutomata"$sf".mp4"
vz0="vid/VKAutomata"$sf"_120hz.mp4"
vz1="vid/VKAutomata"$sf"_144hz.mp4"
vz2="vid/VKAutomata"$sf"_24hz.mp4"
v1="vid/VKAutomata"$sf"_SCALE.mp4"
v2="vid/VKAutomata"$sf"_SMALL.mp4"

v3="vid/VKAutomata"$sf"_4096x2048.mp4"
v4="vid/VKAutomata"$sf"_2048x1024.mp4"
v5="vid/VKAutomata"$sf"_1024x512.mp4"
v6="vid/VKAutomata"$sf"_512x256.mp4"
v7="vid/VKAutomata"$sf"_256x128.mp4"

# --- Bluesky Filenames ---
vbsky1="vid/VKAutomata"$sf"_BSKY_PRIMARY_${scaling_factor}x.mp4" # Scaled from original PAMs
vbsky2="vid/VKAutomata"$sf"_BSKY_FROM_512p.mp4" # Scaled from 512x256 video
vbsky3="vid/VKAutomata"$sf"_BSKY_FROM_256p.mp4" # Scaled from 256x128 video

# --- Consolidated GIF output filepaths/filenames ---
gfull="gif/VKAutomata"$sf"_GFULL.gif"   # Full resolution GIF
gsmall="gif/VKAutomata"$sf"_GSMALL.gif" # Half resolution GIF

# Create MP4 videos (Original Paths)
echo "--- Generating Original Video Formats ---"
# Section A: Different Framerates
buildtime=$(date +%s)
echo "              VID A [1/4] "$v0 $(date +%T)
ffmpeg -loglevel error -y -framerate ${INPUT_ASSUMED_FPS} -i out/IMG%00d.PAM -c:v libx264 -crf 12 -movflags +faststart -pix_fmt yuv420p $v0
echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

# ... (rest of original VID A, B, C sections remain unchanged, ensure $v6 and $v7 are generated before Bluesky paths 2/3) ...

buildtime=$(date +%s)
echo "              VID A [2/4] "$vz0 $(date +%T)
ffmpeg -loglevel error -y -framerate 120 -i out/IMG%00d.PAM -c:v libx264 -crf 12 -movflags +faststart -pix_fmt yuv420p $vz0
echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

buildtime=$(date +%s)
echo "              VID A [3/4] "$vz1 $(date +%T)
ffmpeg -loglevel error -y -framerate 144 -i out/IMG%00d.PAM -c:v libx264 -crf 12 -movflags +faststart -pix_fmt yuv420p $vz1
echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

buildtime=$(date +%s)
echo "              VID A [4/4] "$vz2 $(date +%T)
ffmpeg -loglevel error -y -framerate 24 -i out/IMG%00d.PAM -c:v libx264 -crf 12 -movflags +faststart -pix_fmt yuv420p $vz2
echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

# Section B: Scaled Videos
buildtime=$(date +%s)
echo "              VID B [1/2] "$v2 $(date +%T)
ffmpeg -loglevel error -y -framerate ${INPUT_ASSUMED_FPS} -i out/IMG%00d.PAM -c:v libx264 -crf 12 -movflags +faststart -vf 'scale=iw/2:ih/2:flags=bicubic' -pix_fmt yuv420p $v2
echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

buildtime=$(date +%s)
echo "              VID B [2/2] "$v1 $(date +%T)
ffmpeg -loglevel error -y -framerate ${INPUT_ASSUMED_FPS} -i out/IMG%00d.PAM -c:v libx264 -crf 12 -movflags +faststart -vf 'scale=iw*4:ih*4:flags=neighbor' -pix_fmt yuv420p $v1
echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

# Section C: Fixed Resolutions (Generates $v7 and $v6 needed for Bluesky paths 2/3)
buildtime=$(date +%s)
echo "              VID C [1/5] "$v7 $(date +%T)
ffmpeg -loglevel error -y -framerate ${INPUT_ASSUMED_FPS} -i out/IMG%00d.PAM -s 256x128 -sws_flags bicubic -c:v libx264 -crf 1 -movflags +faststart -pix_fmt yuv420p $v7
echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

buildtime=$(date +%s)
echo "              VID C [2/5] "$v6 $(date +%T)
ffmpeg -loglevel error -y -framerate ${INPUT_ASSUMED_FPS} -i out/IMG%00d.PAM -s 512x256 -sws_flags bicubic -c:v libx264 -crf 1 -movflags +faststart -pix_fmt yuv420p $v6
echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

buildtime=$(date +%s)
echo "              VID C [3/5] "$v5 $(date +%T)
ffmpeg -loglevel error -y -framerate ${INPUT_ASSUMED_FPS} -i out/IMG%00d.PAM -s 1024x512 -sws_flags neighbor -c:v libx264 -crf 14 -movflags +faststart -pix_fmt yuv420p $v5
echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

buildtime=$(date +%s)
echo "              VID C [4/5] "$v4 $(date +%T)
ffmpeg -loglevel error -y -framerate ${INPUT_ASSUMED_FPS} -i out/IMG%00d.PAM -s 2048x1024 -sws_flags neighbor -c:v libx264 -crf 14 -movflags +faststart -pix_fmt yuv420p $v4
echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

buildtime=$(date +%s)
echo "              VID C [5/5] "$v3 $(date +%T)
ffmpeg -loglevel error -y -framerate ${INPUT_ASSUMED_FPS} -i out/IMG%00d.PAM -s 4096x2048 -sws_flags neighbor -c:v libx264 -crf 14 -movflags +faststart -pix_fmt yuv420p $v3
echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))


# --- Create Bluesky Optimized Videos ---
echo "--- Generating Bluesky Optimized Videos (CRF ${BSKY_QUALITY_CRF}) ---"
# Path 1: From Original PAMs with Dynamic Scaling
buildtime=$(date +%s)
echo "           BLUESKY VID [1/3] PRIMARY "$vbsky1 $(date +%T)
ffmpeg -loglevel error -y -framerate ${INPUT_ASSUMED_FPS} -i out/IMG%00d.PAM \
    -vf "select='not(mod(n,${bsky_skip_interval}))',setpts=N/${BSKY_TARGET_FPS}/TB,scale=iw*${scaling_factor}:ih*${scaling_factor}:flags=neighbor" \
    -c:v libx264 -profile:v high -crf ${BSKY_QUALITY_CRF} -pix_fmt yuv420p -r ${BSKY_TARGET_FPS} -movflags +faststart $vbsky1
echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

# Path 2: From 512x256 Video ($v6)
if [ -f "$v6" ]; then
    buildtime=$(date +%s)
    echo "           BLUESKY VID [2/3] FROM 512p "$vbsky2 $(date +%T)
    ffmpeg -loglevel error -y -i "$v6" \
        -vf "select='not(mod(n,${bsky_skip_interval}))',setpts=N/${BSKY_TARGET_FPS}/TB,scale=iw*4:ih*4:flags=neighbor" \
        -c:v libx264 -profile:v high -crf ${BSKY_QUALITY_CRF} -pix_fmt yuv420p -r ${BSKY_TARGET_FPS} -movflags +faststart $vbsky2
    echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))
else
    echo "           BLUESKY VID [2/3] SKIPPED - Input $v6 not found."
fi

# Path 3: From 256x128 Video ($v7)
if [ -f "$v7" ]; then
    buildtime=$(date +%s)
    echo "           BLUESKY VID [3/3] FROM 256p "$vbsky3 $(date +%T)
    ffmpeg -loglevel error -y -i "$v7" \
        -vf "select='not(mod(n,${bsky_skip_interval}))',setpts=N/${BSKY_TARGET_FPS}/TB,scale=iw*8:ih*8:flags=neighbor" \
        -c:v libx264 -profile:v high -crf ${BSKY_QUALITY_CRF} -pix_fmt yuv420p -r ${BSKY_TARGET_FPS} -movflags +faststart $vbsky3
    echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))
else
    echo "           BLUESKY VID [3/3] SKIPPED - Input $v7 not found."
fi

# --- Create GIFs (Consolidated & Fixed) ---
echo "--- Generating GIFs ---"
buildtime=$(date +%s)
echo "              GIF [1/2] SMALL "$gsmall $(date +%T)
ffmpeg -loglevel error -y -framerate ${INPUT_ASSUMED_FPS} -i out/IMG%00d.PAM \
    -filter_complex "[0:v]select='not(mod(n,${gif_skip_interval}))',setpts=N/${GIF_TARGET_FPS}/TB,scale=iw/2:ih/2:flags=neighbor[scaled_frames];[scaled_frames]split[s1][s2];[s1]palettegen=reserve_transparent=0:stats_mode=single[pal];[s2][pal]paletteuse=new=1" \
    -r ${GIF_TARGET_FPS} $gsmall
echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))

buildtime=$(date +%s)
echo "              GIF [2/2] FULL "$gfull $(date +%T)
ffmpeg -loglevel error -y -framerate ${INPUT_ASSUMED_FPS} -i out/IMG%00d.PAM \
    -filter_complex "[0:v]select='not(mod(n,${gif_skip_interval}))',setpts=N/${GIF_TARGET_FPS}/TB[selected_frames];[selected_frames]split[s1][s2];[s1]palettegen=reserve_transparent=0:stats_mode=single[pal];[s2][pal]paletteuse=new=1" \
    -r ${GIF_TARGET_FPS} $gfull
echo "                  Seconds elapsed: "$(($(date +%s)-$buildtime))


# Hope and pray that it worked!
echo "  Video Creation Complete!"
echo "  Total Seconds elapsed: "$(($(date +%s)-$starttime))
echo ""
