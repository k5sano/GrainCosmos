#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

/**
 * Simple level meter component
 */
class LevelMeterComponent : public juce::Component
{
public:
    LevelMeterComponent() = default;
    ~LevelMeterComponent() override = default;

    void setLevel(float newLevel) { level = newLevel; repaint(); }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);

        int barHeight = static_cast<int>(level * getHeight());
        g.setColour(level > 0.9f ? juce::Colours::red :
                      level > 0.7f ? juce::Colours::orange :
                      juce::Colours::green);
        g.fillRect(0, getHeight() - barHeight, getWidth(), barHeight);
    }

private:
    float level = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeterComponent)
};

class ViolinBassAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    ViolinBassAudioProcessorEditor(ViolinBassAudioProcessor&);
    ~ViolinBassAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    ViolinBassAudioProcessor& audioProcessor;

    // UI Components
    juce::Label titleLabel;

    // Sliders
    juce::Slider pitchSlider;
    juce::Slider fineTuneSlider;
    juce::ToggleButton formantToggle;
    juce::Slider dryWetSlider;
    juce::Slider outputGainSlider;
    juce::Slider lpfCutoffSlider;
    juce::Slider hpfCutoffSlider;

    // Labels
    juce::Label pitchLabel;
    juce::Label fineTuneLabel;
    juce::Label formantLabel;
    juce::Label dryWetLabel;
    juce::Label outputGainLabel;
    juce::Label lpfCutoffLabel;
    juce::Label hpfCutoffLabel;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pitchAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> fineTuneAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryWetAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lpfCutoffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hpfCutoffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> formantAttachment;

    // Level meters
    LevelMeterComponent inputMeterComponent;
    LevelMeterComponent outputMeterComponent;
    float inputMeterLevel = 0.0f;
    float outputMeterLevel = 0.0f;

    // Latency display
    juce::Label latencyLabel;

    // Timer for UI updates
    void timerCallback() override;
    void updateLevelMeters();
    void updateLatencyDisplay();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ViolinBassAudioProcessorEditor)
};
