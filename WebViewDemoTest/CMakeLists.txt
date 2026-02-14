cmake_minimum_required(VERSION 3.15)
project(WebViewDemoTest LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find JUCE
set(JUCE_DIR /Applications/JUCE)
if(NOT EXISTS ${JUCE_DIR})
    set(JUCE_DIR /tmp/JUCE-8.0.4)
endif()

if(NOT EXISTS ${JUCE_DIR})
    message(FATAL_ERROR "JUCE not found at /Applications/JUCE or /tmp/JUCE-8.0.4")
endif()

add_subdirectory(${JUCE_DIR})

# Create the plugin
juce_add_plugin("WebViewDemo"
    VERSION "1.0.0"
    COMPANY_NAME "Test"
    IS_SYNTH false
    NEEDS_MIDI_INPUT false
    NEEDS_MIDI_OUTPUT false
    IS_AUV3_WRAPPER false
    COPY_PLUGIN_AFTER_BUILD true
    PLUGIN_FORMATS VST3 Standalone
)

# Target sources
target_sources("WebViewDemo"
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

# Include directories
target_include_directories("WebViewDemo"
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# JUCE modules
juce_use_modules("WebViewDemo"
    juce_audio_basics
    juce_audio_devices
    juce_audio_formats
    juce_audio_plugin_client
    juce_audio_processors
    juce_audio_utils
    juce_core
    juce_data_structures
    juce_dsp
    juce_events
    juce_graphics
    juce_gui_basics
    juce_gui_extra
)
