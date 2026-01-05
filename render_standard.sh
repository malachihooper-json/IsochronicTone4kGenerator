#!/bin/bash

BINARY="./build/IsochronicBatchGen"
OUTPUT_DIR="./renders"
mkdir -p "$OUTPUT_DIR"

PULSE_FREQ=${1:-10.0}
CARRIER_FREQ=${2:-440.0}
SOFTNESS=${3:-0.5}
GAIN_DB=${4:--6.0}

# Durations in seconds
declare -A DURATIONS
DURATIONS["20min"]=1200
DURATIONS["1hour"]=3600
DURATIONS["3hour"]=10800
DURATIONS["5hour"]=18000
DURATIONS["10hour"]=36000
DURATIONS["20hour"]=72000

for NAME in "${!DURATIONS[@]}"; do
    SECONDS=${DURATIONS[$NAME]}
    FILE="$OUTPUT_DIR/isochronic_${NAME}_${PULSE_FREQ}Hz_${CARRIER_FREQ}Hz.wav"
    echo "------------------------------------------------"
    echo "Rendering $NAME ($SECONDS seconds)..."
    $BINARY "$FILE" "$SECONDS" "$PULSE_FREQ" "$CARRIER_FREQ" "$SOFTNESS" "$GAIN_DB"
done

echo "------------------------------------------------"
echo "All renders complete in $OUTPUT_DIR"
