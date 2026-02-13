cmake_minimum_required(VERSION 3.22)

project(GrainCosmos VERSION 1.0.0 LANGUAGES C CXX)

add_subdirectory(JUCE)

juce_add_plugin(GrainCosmos
    COMPANY_NAME "PluginFreedom"
    PLUGIN_MANUFACTURER_CODE Plgf
    PLUGIN_CODE GrCs
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "GrainCosmos"
    NEEDS_WEB_BROWSER FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

# WebView UI is not used currently, so disable binary data linking
# juce_add_binary_data(GrainCosmosData
#     SOURCES
#         Source/ui/public/index.html
#         Source/ui/public/js/juce/index.js
#         Source/ui/public/js/juce/check_native_interop.js
# )

target_sources(GrainCosmos
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_include_directories(GrainCosmos
    PRIVATE
        Source
)

target_link_libraries(GrainCosmos
    PRIVATE
        # GrainCosmosData  ← commented out
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

juce_generate_juce_header(GrainCosmos)

target_compile_definitions(GrainCosmos
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)
