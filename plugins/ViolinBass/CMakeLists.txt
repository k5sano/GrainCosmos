cmake_minimum_required(VERSION 3.22)
project(ViolinBass VERSION 1.0.0 LANGUAGES C CXX)

# Use JUCE from GrainCosmos (same as DroneCosmos)
add_subdirectory(../GrainCosmos/JUCE JUCE)

# Find Rubber Band Library (via Homebrew)
execute_process(COMMAND brew --prefix rubberband
    OUTPUT_VARIABLE RUBBERBAND_PREFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE RUBBERBAND_BREW_RESULT
)

# Fallback to common paths if brew fails
if(NOT RUBBERBAND_BREW_RESULT EQUAL 0)
    set(RUBBERBAND_PREFIX "/opt/homebrew/opt/rubberband")
endif()

set(RUBBERBAND_INCLUDE_DIRS "${RUBBERBAND_PREFIX}/include")
set(RUBBERBAND_LIB_DIRS "${RUBBERBAND_PREFIX}/lib")

find_library(RUBBERBAND_LIBRARY
    NAMES rubberband rubberband库
    PATHS ${RUBBERBAND_LIB_DIRS}
    NO_DEFAULT_PATH
)

if(NOT RUBBERBAND_LIBRARY)
    message(FATAL_ERROR "Rubber Band Library not found. Install via: brew install rubberband")
endif()

message(STATUS "Found Rubber Band: ${RUBBERBAND_LIBRARY}")
message(STATUS "Rubber Band include dirs: ${RUBBERBAND_INCLUDE_DIRS}")

# Create VST3 plugin
juce_add_plugin(ViolinBass
    COMPANY_NAME "K5SANO"
    PLUGIN_MANUFACTURER_CODE Plgf
    PLUGIN_CODE VlBs
    FORMATS VST3 Standalone
    PRODUCT_NAME "ViolinBass"
    NEEDS_WEB_BROWSER FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_DESCRIPTION "Low-latency pitch shifter for violin-to-bass conversion"
)

# Rubber Band Library headers and linking
target_include_directories(ViolinBass
    PRIVATE
        ${RUBBERBAND_INCLUDE_DIRS}
        src
)

target_link_libraries(ViolinBass
    PRIVATE
        ${RUBBERBAND_LIBRARY}
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_sources(ViolinBass
    PRIVATE
        src/PluginProcessor.cpp
        src/PluginEditor.cpp
        src/PitchShifter.cpp
)

juce_generate_juce_header(ViolinBass)

target_compile_definitions(ViolinBass
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_LOG_ASSERTIONS=1
)

# Universal Binary for macOS
if(APPLE)
    set_target_properties(ViolinBass PROPERTIES
        OSX_ARCHITECTURES "arm64;x86_64"
    )
endif()
