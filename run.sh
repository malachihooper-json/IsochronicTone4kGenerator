#!/bin/bash

# Updated run.sh: Fixed bash compatibility and metadata output

BINARY="./build/IsochronicBatchGen"
OUTPUT_DIR="./renders"

# Make sure the directory for the outputs exists
mkdir -p "$OUTPUT_DIR"

# Quick check to make sure the audio engine is built
if [ ! -f "$BINARY" ]; then
    echo "Hang on, the audio engine hasn't been built yet."
    echo "Go back to master.sh and run the 'Build' option first."
    exit 1
fi

echo "------------------------------------------------"
echo "  LET'S GENERATE SOME TONES"
echo "------------------------------------------------"
echo "I'll guide you through a few settings to get your session ready."

# 1. Type
echo "Select Session Type:"
echo "0) Isochronic (Rhythmic Pulses)"
echo "1) Binaural   (Frequency Offsets)"
read -p "Choice [0-1, default 0]: " TYPE
TYPE=${TYPE:-0}
if [ "$TYPE" -eq 0 ]; then TYPE_NAME="Isochronic"; else TYPE_NAME="Binaural"; fi

# 2. Frequencies
read -p "Target Frequency (Hz, e.g. 7.83, 3.0): " PULSE_FREQ
PULSE_FREQ=${PULSE_FREQ:-10.0}

read -p "Carrier Frequency (Hz, default 440.0): " CARRIER_FREQ
CARRIER_FREQ=${CARRIER_FREQ:-440.0}

# Determine State for Contextual Revenue
PULSE_INT=$(echo $PULSE_FREQ | cut -d. -f1)
if [ "$PULSE_INT" -lt 4 ]; then STATE="Delta"; elif [ "$PULSE_INT" -lt 8 ]; then STATE="Theta"; elif [ "$PULSE_INT" -lt 13 ]; then STATE="Alpha"; elif [ "$PULSE_INT" -lt 30 ]; then STATE="Beta"; else STATE="Gamma"; fi

# 3. Income/Revenue Layer
echo "------------------------------------------------"
echo "REVENUE LAYER"
REVENUE_CONFIG="revenue_config.txt"
if [ -f "$REVENUE_CONFIG" ]; then
    STORE_URL=$(grep "STORE_URL=" "$REVENUE_CONFIG" | cut -d= -f2)
    echo "Found Global Store URL: $STORE_URL"
    read -p "Enable Corner QR Code for Digital Downloads? [y/n, default y]: " ENABLE_QR
    ENABLE_QR=${ENABLE_QR:-"y"}
else
    echo "No Global Store URL configured. (Configure in master.sh)"
    ENABLE_QR="n"
fi

if [ "$ENABLE_QR" == "y" ]; then
    if command -v qrencode &> /dev/null; then
        echo "Generating Session QR Code..."
        qrencode -o session_qr.png "$STORE_URL"
        QR_PATH="session_qr.png"
    else
        echo "Note: 'qrencode' not found. Skip QR overlay."
        QR_PATH=""
    fi
else
    QR_PATH=""
fi

# 4. Brand Stamp
read -p "Include Author/Brand Stamp? (blank for none): " BRAND_NAME

# 5. Duration
echo "------------------------------------------------"
echo "Select Duration:"
echo "1) 20 mins     4) 5 hours"
echo "2) 1 hour      5) 10 hours"
echo "3) 3 hours     6) 20 hours"
read -p "Choice [1-6, default 2]: " DUR_CHOICE

case "$DUR_CHOICE" in
    1) DUR_NAME="20min" ;;
    2) DUR_NAME="1hour" ;;
    3) DUR_NAME="3hour" ;;
    4) DUR_NAME="5hour" ;;
    5) DUR_NAME="10hour" ;;
    6) DUR_NAME="20hour" ;;
    *) DUR_NAME="1hour" ;;
esac

echo "------------------------------------------------"
echo "INITIALIZING PRODUCTION"
echo "Mode: $TYPE_NAME | $PULSE_FREQ Hz ($STATE)"
echo "------------------------------------------------"

./render_video.sh "$PULSE_FREQ" "$CARRIER_FREQ" "0.5" "-10.0" "$DUR_NAME" "$TYPE" "0" "" "-1" "0.0" "$BRAND_NAME" "$QR_PATH"

# Bash compatibility for uppercase (Mac default bash is old)
UPPER_TYPE=$(echo "$TYPE_NAME" | tr '[:lower:]' '[:upper:]')
UPPER_DUR=$(echo "$DUR_NAME" | tr '[:lower:]' '[:upper:]')

echo "------------------------------------------------"
echo "METADATA GENERATED"
echo "------------------------------------------------"
echo "Title: PURE ${PULSE_FREQ}Hz ${UPPER_TYPE} TONE - [${UPPER_DUR}]"
echo ""
echo "Description:"
echo "This session is tuned to ${PULSE_FREQ}Hz, a frequency associated with ${STATE}."
echo ""
if [ -n "$STORE_URL" ]; then
    echo "GET THE LOSSLESS VERSION:"
    echo "Download high-fidelity 24-bit 10-hour audio here: $STORE_URL"
    echo ""
fi
echo "RECOMMENDED FOR THIS SESSION:"
# Fetch affiliate links from python helper
python3 get_affiliates.py "$STATE" "products.json"
echo ""
echo "TECHNICAL SPECIFICATIONS:"
echo "- Resonance: ${PULSE_FREQ}Hz"
echo "- Carrier: ${CARRIER_FREQ}Hz"
echo "- Rendering: 4K Ultra HD"
echo "------------------------------------------------"
echo "Done!"
