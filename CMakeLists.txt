cmake_minimum_required(VERSION 3.15)

project(IsochronicToneGen VERSION 1.0.0)

# We pull in JUCE directly from GitHub so we don't have to manage it locally.
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        master 
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(IsochronicToneGen
    COMPANY_NAME "Antigravity"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_STEP_AFTER_BUILD FALSE
    PRODUCT_NAME "IsochronicToneGen"
    FORMATS VST3 AU
)

target_sources(IsochronicToneGen
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
)

target_compile_definitions(IsochronicToneGen
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(IsochronicToneGen
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_extra
        juce::juce_gui_basics
        juce::juce_graphics
        juce::juce_events
        juce::juce_core
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# This is the command line tool for rendering long sessions.
add_executable(IsochronicBatchGen
    BatchGenerator/gen_audio.cpp
)

target_link_libraries(IsochronicBatchGen
    PRIVATE
        juce::juce_audio_formats
        juce::juce_audio_basics
        juce::juce_core
    PUBLIC
        juce::juce_recommended_config_flags
)
