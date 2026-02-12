#include "PluginEditor.h"

//==============================================================================
// Constructor - CRITICAL: Initialize in correct order
//==============================================================================

GrainCosmosAudioProcessorEditor::GrainCosmosAudioProcessorEditor(GrainCosmosAudioProcessor& p)
    : AudioProcessorEditor(p), audioProcessor(p)
{
    // ========================================================================
    // INITIALIZATION SEQUENCE (CRITICAL ORDER)
    // ========================================================================
    //
    // 1. Create relays FIRST (before WebView construction)
    // 2. Create WebView with relay options
    // 3. Create parameter attachments LAST (after WebView construction)
    //
    // This matches member declaration order and ensures safe destruction.
    // ========================================================================

    // ------------------------------------------------------------------------
    // STEP 1: CREATE RELAYS (before WebView!)
    // ------------------------------------------------------------------------
    //
    // Each relay bridges a C++ parameter to JavaScript state.
    // Relay constructor takes parameter ID (must match APVTS).
    //

    // Slider relays (8 parameters)
    delayTimeRelay = std::make_unique<juce::WebSliderRelay>("delay_time");
    grainSizeRelay = std::make_unique<juce::WebSliderRelay>("grain_size");
    envelopeShapeRelay = std::make_unique<juce::WebSliderRelay>("envelope_shape");
    distortionAmountRelay = std::make_unique<juce::WebSliderRelay>("distortion_amount");
    feedbackRelay = std::make_unique<juce::WebSliderRelay>("feedback");
    chaosRelay = std::make_unique<juce::WebSliderRelay>("chaos");
    characterRelay = std::make_unique<juce::WebSliderRelay>("character");
    mixRelay = std::make_unique<juce::WebSliderRelay>("mix");

    // Toggle relays (2 parameters)
    freezeRelay = std::make_unique<juce::WebToggleButtonRelay>("freeze");
    tempoSyncRelay = std::make_unique<juce::WebToggleButtonRelay>("tempo_sync");

    // ------------------------------------------------------------------------
    // STEP 2: CREATE WEBVIEW (with relay options)
    // ------------------------------------------------------------------------
    //
    // WebView creation with all necessary options:
    // - withNativeIntegrationEnabled() - REQUIRED for JUCE parameter binding
    // - withResourceProvider() - REQUIRED for JUCE 8 (serves embedded files)
    // - withOptionsFrom(*relay) - REQUIRED for each parameter relay
    // - withKeepPageLoadedWhenBrowserIsHidden() - OPTIONAL (FL Studio fix)
    //
    webView = std::make_unique<juce::WebBrowserComponent>(
        juce::WebBrowserComponent::Options{}
            // REQUIRED: Enable JUCE frontend library
            .withNativeIntegrationEnabled()

            // REQUIRED: Resource provider for embedded files
            .withResourceProvider([this](const auto& url) {
                return getResource(url);
            })

            // OPTIONAL: FL Studio fix (prevents blank screen on focus loss)
            // Remove if not targeting FL Studio
            .withKeepPageLoadedWhenBrowserIsHidden()

            // REQUIRED: Register each relay with WebView
            // This creates JavaScript state objects accessible via:
            // - Juce.getSliderState("PARAM_ID")
            // - Juce.getToggleState("PARAM_ID")
            //
            // Slider relays (8 parameters)
            .withOptionsFrom(*delayTimeRelay)
            .withOptionsFrom(*grainSizeRelay)
            .withOptionsFrom(*envelopeShapeRelay)
            .withOptionsFrom(*distortionAmountRelay)
            .withOptionsFrom(*feedbackRelay)
            .withOptionsFrom(*chaosRelay)
            .withOptionsFrom(*characterRelay)
            .withOptionsFrom(*mixRelay)

            // Toggle relays (2 parameters)
            .withOptionsFrom(*freezeRelay)
            .withOptionsFrom(*tempoSyncRelay)
    );

    // ------------------------------------------------------------------------
    // STEP 3: CREATE PARAMETER ATTACHMENTS (after WebView!)
    // ------------------------------------------------------------------------
    //
    // Attachments synchronize APVTS parameters with relay state.
    // Constructor: (parameter, relay, undoManager)
    //
    // Parameter must be retrieved from APVTS:
    //   audioProcessor.apvts.getParameter("PARAM_ID")
    //
    // Undo manager typically nullptr for real-time parameters.
    //

    // Slider attachments (8 parameters)
    delayTimeAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *audioProcessor.apvts.getParameter("delay_time"),
        *delayTimeRelay,
        nullptr  // No undo manager
    );
    grainSizeAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *audioProcessor.apvts.getParameter("grain_size"),
        *grainSizeRelay,
        nullptr
    );
    envelopeShapeAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *audioProcessor.apvts.getParameter("envelope_shape"),
        *envelopeShapeRelay,
        nullptr
    );
    distortionAmountAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *audioProcessor.apvts.getParameter("distortion_amount"),
        *distortionAmountRelay,
        nullptr
    );
    feedbackAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *audioProcessor.apvts.getParameter("feedback"),
        *feedbackRelay,
        nullptr
    );
    chaosAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *audioProcessor.apvts.getParameter("chaos"),
        *chaosRelay,
        nullptr
    );
    characterAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *audioProcessor.apvts.getParameter("character"),
        *characterRelay,
        nullptr
    );
    mixAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *audioProcessor.apvts.getParameter("mix"),
        *mixRelay,
        nullptr
    );

    // Toggle attachments (2 parameters)
    freezeAttachment = std::make_unique<juce::WebToggleButtonParameterAttachment>(
        *audioProcessor.apvts.getParameter("freeze"),
        *freezeRelay,
        nullptr
    );
    tempoSyncAttachment = std::make_unique<juce::WebToggleButtonParameterAttachment>(
        *audioProcessor.apvts.getParameter("tempo_sync"),
        *tempoSyncRelay,
        nullptr
    );

    // ------------------------------------------------------------------------
    // WEBVIEW SETUP
    // ------------------------------------------------------------------------

    // Navigate to root (loads index.html via resource provider)
    // JUCE 8 pattern: Use resource provider root instead of data URLs
    webView->goToURL(juce::WebBrowserComponent::getResourceProviderRoot());

    // Make WebView visible
    addAndMakeVisible(*webView);

    // ------------------------------------------------------------------------
    // WINDOW SIZE (from mockup v3 specification)
    // ------------------------------------------------------------------------
    setSize(800, 500);
}

//==============================================================================
// Destructor
//==============================================================================

GrainCosmosAudioProcessorEditor::~GrainCosmosAudioProcessorEditor()
{
    // Members are automatically destroyed in reverse order of declaration:
    // 1. Attachments destroyed first (stop calling evaluateJavascript)
    // 2. WebView destroyed second (safe, attachments are gone)
    // 3. Relays destroyed last (safe, nothing using them)
    //
    // No manual cleanup needed if member order is correct!
}

//==============================================================================
// AudioProcessorEditor Overrides
//==============================================================================

void GrainCosmosAudioProcessorEditor::paint(juce::Graphics& g)
{
    // WebView fills entire editor, so typically no custom painting needed
    // Uncomment if you want a background color visible before WebView loads:
    // g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void GrainCosmosAudioProcessorEditor::resized()
{
    // Make WebView fill the entire editor bounds
    webView->setBounds(getLocalBounds());
}

//==============================================================================
// Resource Provider (JUCE 8 Required Pattern)
//==============================================================================

std::optional<juce::WebBrowserComponent::Resource>
GrainCosmosAudioProcessorEditor::getResource(const juce::String& url)
{
    // ========================================================================
    // RESOURCE PROVIDER IMPLEMENTATION
    // ========================================================================
    //
    // Maps URLs to embedded binary data (from juce_add_binary_data).
    //
    // File path → BinaryData symbol:
    // - ui/public/index.html       → BinaryData::index_html
    // - ui/public/js/juce/index.js → BinaryData::juice_index_js
    //
    // Pattern: Remove "ui/public/", replace "/" with "_", replace "." with "_"
    // ========================================================================

    auto makeVector = [](const char* data, int size) {
        return std::vector<std::byte>(
            reinterpret_cast<const std::byte*>(data),
            reinterpret_cast<const std::byte*>(data) + size
        );
    };

    // Handle root URL (redirect to index.html)
    if (url == "/" || url == "/index.html") {
        return juce::WebBrowserComponent::Resource {
            makeVector(BinaryData::index_html, BinaryData::index_htmlSize),
            "text/html"
        };
    }

    // JUCE frontend library
    if (url == "/js/juce/index.js") {
        return juce::WebBrowserComponent::Resource {
            makeVector(BinaryData::juce_index_js, BinaryData::juce_index_jsSize),
            "text/javascript"
        };
    }

    // Optional: Add CSS, images, or other resources here
    // if (url == "/css/styles.css") {
    //     return juce::WebBrowserComponent::Resource {
    //         makeVector(BinaryData::styles_css, BinaryData::styles_cssSize),
    //         "text/css"
    //     };
    // }

    // 404 - Resource not found
    return std::nullopt;
}
