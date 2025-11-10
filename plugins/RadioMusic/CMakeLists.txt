cmake_minimum_required(VERSION 3.15)

# RadioMusic - MIDI-triggered Sampler
project(RadioMusic VERSION 1.0.0)

# JUCE is located at /Applications/JUCE
add_subdirectory(/Applications/JUCE JUCE)

# Plugin formats: VST3, AU, Standalone
juce_add_plugin(RadioMusic
    COMPANY_NAME "YourCompany"
    PLUGIN_MANUFACTURER_CODE Manu
    PLUGIN_CODE RdMu
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "RadioMusic"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
)

# Source files
target_sources(RadioMusic
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

# Include paths
target_include_directories(RadioMusic
    PRIVATE
        Source
)

# Required JUCE modules
target_link_libraries(RadioMusic
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Compile definitions
target_compile_definitions(RadioMusic
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)
