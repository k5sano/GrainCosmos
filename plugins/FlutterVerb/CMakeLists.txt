cmake_minimum_required(VERSION 3.22)

project(FlutterVerb VERSION 1.0.0 LANGUAGES C CXX)

add_subdirectory(JUCE)

juce_add_plugin(FlutterVerb
    COMPANY_NAME "LexDev"
    PLUGIN_MANUFACTURER_CODE Lexd
    PLUGIN_CODE Fltv
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "FlutterVerb"
    NEEDS_WEB_BROWSER FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

# WebView UI is not used currently, so disable binary data linking
# juce_add_binary_data(FlutterVerb_UIResources
#     SOURCES
#         Source/ui/public/index.html
#         Source/ui/public/js/juce/index.js
#         Source/ui/public/js/juce/check_native_interop.js
# )

target_sources(FlutterVerb
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_include_directories(FlutterVerb
    PRIVATE
        Source
)

target_link_libraries(FlutterVerb
    PRIVATE
        # FlutterVerb_UIResources  ← commented out
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

juce_generate_juce_header(FlutterVerb)

target_compile_definitions(FlutterVerb
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)
