#!/bin/bash

# syndicate.sh: The Wisdom Syndicate Production Controller v1.1

BINARY="./build/IsochronicBatchGen"
OUTPUT_DIR="./renders"
mkdir -p "$OUTPUT_DIR"

echo "===================================================="
echo "    THE WISDOM SYNDICATE - PRODUCTION"
echo "===================================================="

# 1. Select the Wisdom Source
echo "Select Philosophical Source:"
echo "1) Meditations - Marcus Aurelius"
echo "2) The Discourses - Epictetus"
echo "3) Tao Te Ching - Lao Tzu"
echo "4) The Prophet - Kahlil Gibran"
read -p "Choice [1-4, default 1]: " SOURCE_CHOICE

case "$SOURCE_CHOICE" in
    1) BOOK_ID="68"; TITLE="Meditations"; AUTHOR="Marcus Aurelius" ;;
    2) BOOK_ID="14588"; TITLE="Discourses"; AUTHOR="Epictetus" ;;
    3) BOOK_ID="216"; TITLE="Tao Te Ching"; AUTHOR="Lao Tzu" ;;
    4) BOOK_ID="58585"; TITLE="The Prophet"; AUTHOR="Kahlil Gibran" ;;
    *) BOOK_ID="68"; TITLE="Meditations"; AUTHOR="Marcus Aurelius" ;;
esac

# 2. Fetch and Clean
python3 fetch_classics.py "$BOOK_ID" "$TITLE"

# 3. Generate High-Quality Narration (Conservative Mode: macOS 'say')
echo "------------------------------------------------"
echo "Step 1: Generating Narration..."
# We take the first 5 mins for this demo to be fast
head -c 5000 wisdom_text.txt > narration_input.txt
say -v "Daniel" -f narration_input.txt -o narration_temp.aiff
ffmpeg -y -i narration_temp.aiff -ar 44100 narration_final.wav > /dev/null 2>&1
rm narration_temp.aiff narration_input.txt

# Get duration of narration to match the tone
SEC=$(ffprobe -i narration_final.wav -show_entries format=duration -v quiet -of csv='p=0' | cut -d. -f1)

# 4. Select Brainwave Sync
echo "------------------------------------------------"
echo "Step 2: Selecting Cognitive Layer..."
echo "0) Delta (4Hz) - Philosophical Sleep"
echo "1) Alpha (10Hz) - Accelerated Learning"
echo "2) Theta (6Hz) - Deep Contemplation"
read -p "Choice [0-2, default 1]: " B_STATE

case "$B_STATE" in
    0) FREQ="4.0"; NAME="Philosophical Sleep" ;;
    1) FREQ="10.0"; NAME="Deep Learning" ;;
    2) FREQ="6.0"; NAME="Contemplation" ;;
    *) FREQ="10.0"; NAME="Deep Learning" ;;
esac

# 5. Generate Tones
echo "------------------------------------------------"
echo "Step 3: Engineering Background Tones..."
$BINARY "tones_temp.wav" "$SEC" "$FREQ" "440.0" "0.5" "1" "-15.0" "-1" "0.0"

# 6. Render Final Video (Minimalist 4K + Narration + Tones)
echo "------------------------------------------------"
echo "Step 4: Mastering 4K Meditative Audiobook..."
# Remove spaces for filename compatibility
CLEAN_TITLE=$(echo $TITLE | sed 's/ /_/g')
CLEAN_NAME=$(echo $NAME | sed 's/ /_/g')
MP4_FILE="$OUTPUT_DIR/${CLEAN_TITLE}_${CLEAN_NAME}.mp4"

# Safe strings for FFmpeg filters (No : or |)
TEXT="$TITLE by $AUTHOR"
METRICS="Mode $NAME - Frequency ${FREQ}Hz"

ffmpeg -y -f lavfi -i color=c=black:s=3840x2160:r=24 -i narration_final.wav -i tones_temp.wav \
    -filter_complex "[1:a][2:a]amix=inputs=2:duration=first:dropout_transition=2[aout];[0:v]drawtext=text='$TEXT':fontcolor=white:fontsize=120:x=(w-text_w)/2:y=(h-text_h)/2-100,drawtext=text='$METRICS':fontcolor=white@0.4:fontsize=50:x=(w-text_w)/2:y=(h-text_h)/2+100[vout]" \
    -map "[vout]" -map "[aout]" \
    -c:v libx264 -preset fast -tune stillimage -crf 22 -pix_fmt yuv420p \
    -c:a aac -b:a 320k "$MP4_FILE"

# Cleanup
rm tones_temp.wav narration_final.wav wisdom_text.txt
echo "------------------------------------------------"
echo "PRODUCTION COMPLETE"
echo "Video saved to: $MP4_FILE"
echo "------------------------------------------------"
