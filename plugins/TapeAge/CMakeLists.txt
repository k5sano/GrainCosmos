cmake_minimum_required(VERSION 3.15)

# Create the plugin target
juce_add_plugin(TapeAge
    COMPANY_NAME "PluginFreedom"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Plfr
    PLUGIN_CODE Tpag
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "TapeAge")

# Add source files
target_sources(TapeAge
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)

# Link JUCE modules
target_link_libraries(TapeAge
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# Compile definitions
target_compile_definitions(TapeAge
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)
