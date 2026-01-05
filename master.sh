#!/bin/bash

# master.sh: The Command Center for your Automation Business v1.1

# Check for dependencies
if ! command -v ffmpeg &> /dev/null; then
    echo "Wait, it looks like you don't have FFmpeg installed."
    echo "You need FFmpeg to render the audio and video files."
    echo "On Mac, you can usually install it with: brew install ffmpeg"
    exit 1
fi

# Check if the binary actually exists
if [ ! -f build/IsochronicBatchGen ]; then
    echo "I didn't find the generator binary in build/IsochronicBatchGen."
    echo "You probably need to select 'Build/Update Project Assets' from the menu below first."
    echo "------------------------------------------------"
fi

clear
echo "===================================================="
echo "    TONE GENERATION CONTROL CENTER"
echo "===================================================="
echo "  [System Status: Ready]"
echo "===================================================="

PS3='Select Business Module: '
options=(
    "Run Pure Utility Generator (Brainwaves)"
    "Run The Wisdom Syndicate (Audiobooks)"
    "Configure Revenue Layer (QR/Affiliates)"
    "Build/Update Project Assets"
    "View Revenue Roadmap"
    "Exit"
)

select opt in "${options[@]}"
do
    case $opt in
        "Run Pure Utility Generator (Brainwaves)")
            ./run.sh
            break
            ;;
        "Run The Wisdom Syndicate (Audiobooks)")
            ./syndicate.sh
            break
            ;;
        "Configure Revenue Layer (QR/Affiliates)")
            echo "------------------------------------------------"
            read -p "Enter Target Store/Patreon URL: " STORE_URL
            echo "STORE_URL=$STORE_URL" > revenue_config.txt
            echo "Config Saved."
            break
            ;;
        "Build/Update Project Assets")
            echo "Building C++ Engines..."
            mkdir -p build && cd build && cmake .. && make -j8 && cd ..
            echo "Build Complete."
            break
            ;;
        "View Revenue Roadmap")
            cat "/Users/malachihooper/.gemini/antigravity/brain/50128ded-7e77-41b0-b0aa-99017380b18d/income_roadmap.md"
            read -p "Press Enter to return..."
            ./master.sh
            exit
            ;;
        "Exit")
            echo "Shutting down..."
            exit
            ;;
        *) echo "Invalid option $REPLY";;
    esac
done
