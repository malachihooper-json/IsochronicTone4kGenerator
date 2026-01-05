# Isochronic Tone Gen

This project is a set of tools for creating isochronic tones. It uses digital signal processing to generate the rhythmic pulses used in brainwave entrainment. Basically, we take a carrier frequency and pulse its volume at specific intervals to hit target brainwave states.

## How it works

The engine is built on JUCE. It handles the oscillators and the math for the amplitude modulation. 

### Core Components

*   **Carrier Oscillator**: A standard sine or complex wave that sits at your base frequency (like 440Hz).
*   **Pulse Modulation**: This is the "isochronic" part. We modulate the volume of the carrier to create the rhythmic pulses.
*   **Batch Utility**: A C++ tool designed to render long sessions (even 10-20 hours) without hogging system resources. It's much faster than real-time.

## Using the suite

We've automated most of the production steps with shell scripts. You can pick your target frequency (Delta, Theta, etc.) and the scripts handle the rest.

*   **Automated Metadata**: The system calculates the brainwave state based on your pulse frequency and generates the metadata for you.
*   **Revenue Layer**: There's built-in support for adding store links or QR codes directly into the output if you're producing these for distribution.
*   **High Quality**: Everything renders to high-fidelity audio, suitable for lossless distribution.

## Running the scripts (for non-techies)

If you're not used to working in a terminal, don't worry. It's pretty straightforward once you get the hang of it.

First, you'll need to open your terminal (Terminal on Mac, or a bash-compatible shell on Windows). Navigate to this folder.

To actually start the control center, type:
```bash
./master.sh
```

**If it says "permission denied":**
Sometimes the computer doesn't know these scripts are allowed to "run" yet. You can fix that by typing:
```bash
chmod +x *.sh
```
This tells the system that it's okay to execute any file ending in `.sh` in this folder.

### What do the scripts do?
*   **master.sh**: This is your main dashboard. Start here if you want to build the project or run the generation tools.
*   **run.sh**: This handles the tone generation. It'll ask you what frequency you want and how long the session should be.
*   **syndicate.sh**: This is for producing audiobooks with background tones. It downloads classic texts and mixes them with the audio.

## Prerequisites

You'll need a few things installed for everything to work:
1.  **FFmpeg**: This is the "engine" that handles the audio and video rendering. 
2.  **CMake**: Used to build the C++ parts of the project.
3.  **Python 3**: Used for fetching text and managing affiliate links.

## Tech Stack

We use CMake for the build system. It fetches JUCE automatically so you don't have to worry about manual installation.

### Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Note

This software is for research and experimental use. If you're using these for clinical purposes, make sure you're familiar with the neuro-acoustic research behind it.
