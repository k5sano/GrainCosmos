#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * TapeAge Plugin Editor with WebView UI (v3 - Vintage Hardware Aesthetic)
 *
 * This editor uses JUCE's WebBrowserComponent to display an HTML/CSS/JS UI.
 * The WebView communicates with the C++ backend via window.juce.postMessage().
 */
class TapeAgeAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    TapeAgeAudioProcessorEditor(TapeAgeAudioProcessor&);
    ~TapeAgeAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    // Timer callback for updating VU meter
    void timerCallback() override;

    // Message handler for WebView -> C++ communication
    void handleWebViewMessage(const juce::var& message);

    // Send parameter updates to WebView
    void updateWebViewParameter(const juce::String& paramID, float value);

    // Send VU meter level to WebView
    void updateVUMeter(float dbLevel);

    //==============================================================================
    TapeAgeAudioProcessor& audioProcessor;

    // WebView component
    std::unique_ptr<juce::WebBrowserComponent> webView;

    // Parameter attachments for synchronization
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ageAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    // VU meter state
    float currentVULevel = -60.0f;
    float peakVULevel = -60.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TapeAgeAudioProcessorEditor)
};
