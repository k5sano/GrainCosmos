cmake_minimum_required(VERSION 3.22)

juce_add_plugin(RadioMusic
    COMPANY_NAME "LexAudio"
    PLUGIN_MANUFACTURER_CODE LxAu
    PLUGIN_CODE RdMu
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "RadioMusic"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Fx Sampler
    PLIST_TO_MERGE "${CMAKE_CURRENT_SOURCE_DIR}/Info-additions.plist")

target_sources(RadioMusic PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

juce_generate_juce_header(RadioMusic)

target_link_libraries(RadioMusic PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)

target_compile_definitions(RadioMusic PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0)
