#include "PluginEditor.h"

//==============================================================================
TapeAgeAudioProcessorEditor::TapeAgeAudioProcessorEditor(TapeAgeAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Set editor size (matches HTML window dimensions)
    setSize(500, 450);

    // Create WebView component
    webView = std::make_unique<juce::WebBrowserComponent>();
    addAndMakeVisible(webView.get());

    // Load HTML file from binary data
    // Note: You'll need to add the HTML file to CMakeLists.txt as a binary resource
    juce::String htmlContent = juce::String::fromUTF8(
        BinaryData::v3ui_html,
        BinaryData::v3ui_htmlSize
    );

    // Load HTML into WebView
    webView->goToURL("data:text/html;charset=utf-8," + juce::URL::addEscapeChars(htmlContent, false));

    // Set up message handler for WebView -> C++ communication
    webView->pageLoadHadNetworkError = [this](const juce::String& error) {
        DBG("WebView network error: " + error);
    };

    // Start timer for VU meter updates (60 Hz)
    startTimerHz(60);
}

TapeAgeAudioProcessorEditor::~TapeAgeAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void TapeAgeAudioProcessorEditor::paint(juce::Graphics& g)
{
    // WebView fills entire editor, no need to paint background
}

void TapeAgeAudioProcessorEditor::resized()
{
    // WebView fills entire editor bounds
    if (webView != nullptr)
        webView->setBounds(getLocalBounds());
}

//==============================================================================
void TapeAgeAudioProcessorEditor::timerCallback()
{
    // Update VU meter with current output level
    float outputLevel = audioProcessor.getOutputLevel();
    updateVUMeter(outputLevel);

    // Check for parameter changes and sync to WebView
    auto& params = audioProcessor.getParameters();
    for (auto* param : params)
    {
        if (auto* audioParam = dynamic_cast<juce::AudioProcessorParameter*>(param))
        {
            float value = audioParam->getValue();
            juce::String paramID = audioParam->getName(100);
            updateWebViewParameter(paramID, value);
        }
    }
}

void TapeAgeAudioProcessorEditor::handleWebViewMessage(const juce::var& message)
{
    // Parse message from JavaScript
    if (!message.isObject())
        return;

    auto obj = message.getDynamicObject();
    if (obj == nullptr)
        return;

    juce::String type = obj->getProperty("type").toString();

    if (type == "parameterChange")
    {
        juce::String paramID = obj->getProperty("parameter").toString();
        float value = static_cast<float>(obj->getProperty("value"));

        // Update parameter in processor
        if (auto* param = audioProcessor.getParameters().getParameter(paramID))
        {
            param->setValueNotifyingHost(value);
        }
    }
}

void TapeAgeAudioProcessorEditor::updateWebViewParameter(const juce::String& paramID, float value)
{
    if (webView == nullptr)
        return;

    // Call JavaScript function to update UI
    juce::String jsCode = "if (typeof setParameterValue === 'function') { "
                          "setParameterValue('" + paramID + "', " + juce::String(value) + "); "
                          "}";

    webView->evaluateJavascript(jsCode);
}

void TapeAgeAudioProcessorEditor::updateVUMeter(float dbLevel)
{
    if (webView == nullptr)
        return;

    // Smooth VU meter movement
    currentVULevel = currentVULevel * 0.8f + dbLevel * 0.2f;

    // Peak hold
    if (dbLevel > peakVULevel)
        peakVULevel = dbLevel;
    else
        peakVULevel *= 0.99f; // Slow decay

    // Call JavaScript function to update VU meter
    juce::String jsCode = "if (typeof setVUMeterLevel === 'function') { "
                          "setVUMeterLevel(" + juce::String(currentVULevel) + "); "
                          "}";

    webView->evaluateJavascript(jsCode);
}
