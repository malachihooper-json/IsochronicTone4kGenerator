#!/bin/bash

# Fixed render_video.sh: Better text handling and bash compatibility

if ! command -v ffmpeg &> /dev/null; then
    echo "Error: ffmpeg is not installed."
    exit 1
fi

BINARY="./build/IsochronicBatchGen"
OUTPUT_DIR="./renders"
mkdir -p "$OUTPUT_DIR"

PULSE_FREQ=${1:-10.0}
CARRIER_FREQ=${2:-440.0}
SOFTNESS=${3:-0.5}
GAIN_DB=${4:--10.0}
DURATION_NAME=${5:-"1hour"}
TYPE=${6:-0} 
THEME=${7:-0} 
BG_IMG=${8:-""}
NOISE_TYPE=${9:--1}
NOISE_LEVEL=${10:-0.0}
BRAND_NAME=${11:-""}
QR_CODE_PATH=${12:-""}

case "$DURATION_NAME" in
    "20min")  SECONDS=1200 ;;
    "1hour")  SECONDS=3600 ;;
    "3hour")  SECONDS=10800 ;;
    "5hour")  SECONDS=18000 ;;
    "10hour") SECONDS=36000 ;;
    "20hour") SECONDS=72000 ;;
    *) 
        if [[ "$DURATION_NAME" =~ ^[0-9]+$ ]]; then SECONDS=$DURATION_NAME; else SECONDS=3600; fi
        ;;
esac

WAV_FILE="$OUTPUT_DIR/temp_audio.wav"
if [ "$TYPE" -eq 0 ]; then TYPE_NAME="Isochronic"; else TYPE_NAME="Binaural"; fi
MP4_FILE="$OUTPUT_DIR/${TYPE_NAME}_${DURATION_NAME}_${PULSE_FREQ}Hz.mp4"

# Bash compatibility: avoid ${VAR^^} for old versions
UPPER_TYPE=$(echo "$TYPE_NAME" | tr '[:lower:]' '[:upper:]')

echo "Step 1: Engineering Pure Mastered Audio..."
$BINARY "$WAV_FILE" "$SECONDS" "$PULSE_FREQ" "$CARRIER_FREQ" "$SOFTNESS" "$TYPE" "$GAIN_DB" "$NOISE_TYPE" "$NOISE_LEVEL"

if [ ! -s "$WAV_FILE" ]; then
    echo "Error: Audio generation failed."
    exit 1
fi

echo "Step 2: Rendering Visuals..."

TEXT="PURE ${PULSE_FREQ}Hz ${UPPER_TYPE} TONE"

# We'll build the filter complex step by step
# 0:v is the black background
# 1:v is the QR code if it exists, otherwise 1:a is the audio
V_FILTER="[0:v]drawtext=text='$TEXT':fontcolor=white:fontsize=120:x=(w-text_w)/2:y=(h-text_h)/2"

if [ -n "$BRAND_NAME" ]; then
    V_FILTER="$V_FILTER,drawtext=text='$BRAND_NAME':fontcolor=white@0.4:fontsize=40:x=(w-text_w)/2:y=h-200"
fi

if [ -f "$QR_CODE_PATH" ]; then
    # With QR: input 1 is the image, input 2 is the audio
    FF_INPUTS="-f lavfi -i color=c=black:s=3840x2160:r=24 -i $QR_CODE_PATH -i $WAV_FILE"
    # Scale and overlay QR [1:v]
    V_COMPLEX="$V_FILTER[base];[1:v]scale=300:300,format=rgba,colorchannelmixer=aa=0.5[qr];[base][qr]overlay=W-w-100:H-h-100"
    A_MAP="-map 2:a"
else
    # Without QR: input 1 is the audio
    FF_INPUTS="-f lavfi -i color=c=black:s=3840x2160:r=24 -i $WAV_FILE"
    V_COMPLEX="$V_FILTER"
    A_MAP="-map 1:a"
fi

ffmpeg -y $FF_INPUTS -filter_complex "$V_COMPLEX" \
    -map 0:v $A_MAP \
    -c:v libx264 -preset ultrafast -tune stillimage -crf 22 -pix_fmt yuv420p \
    -c:a aac -b:a 320k -shortest "$MP4_FILE"

rm "$WAV_FILE"
echo "Production Complete: $MP4_FILE"
