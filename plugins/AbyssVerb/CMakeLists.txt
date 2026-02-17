cmake_minimum_required(VERSION 3.22)

project(AbyssVerb VERSION 1.0.0 LANGUAGES C CXX)

add_subdirectory(/Applications/JUCE ${CMAKE_BINARY_DIR}/JUCE)

juce_add_plugin(AbyssVerb
    COMPANY_NAME "K5SANO"
    PLUGIN_MANUFACTURER_CODE K5sn
    PLUGIN_CODE Abys
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "AbyssVerb"
    NEEDS_WEB_BROWSER FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

target_sources(AbyssVerb
    PRIVATE
        src/PluginProcessor.cpp
        src/PluginEditor.cpp
)

target_include_directories(AbyssVerb
    PRIVATE
        src
)

target_link_libraries(AbyssVerb
    PRIVATE
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

juce_generate_juce_header(AbyssVerb)

target_compile_definitions(AbyssVerb
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)
