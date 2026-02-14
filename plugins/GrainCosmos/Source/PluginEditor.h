#pragma once

#include "PluginProcessor.h"
#include "XYPadComponent.h"

class GrainCosmosAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit GrainCosmosAudioProcessorEditor(GrainCosmosAudioProcessor&);
    ~GrainCosmosAudioProcessorEditor() override;

    void resized() override;
    void paint(juce::Graphics& g) override;
    void loadPreset(int presetId);
    void saveUserPreset();
    void scanUserPresets();
    void loadUserPreset(const juce::File& file);
    juce::File getPresetsFolder();

private:
    GrainCosmosAudioProcessor& processorRef;

    std::unique_ptr<XYPadPanel> xyPad1;
    std::unique_ptr<XYPadPanel> xyPad2;
    std::unique_ptr<XYPadPanel> xyPad3;
    std::unique_ptr<XYPadPanel> xyPad4;

    juce::Slider mixSlider;
    juce::Slider distortionSlider;
    juce::Slider volumeSlider;
    juce::Slider thresholdSlider;
    juce::Slider releaseSlider;

    juce::ComboBox presetSelector;
    juce::TextButton savePresetButton;
    juce::StringArray userPresetNames;
    juce::Array<juce::File> userPresetFiles;
    int factoryPresetCount = 6;

    juce::Label titleLabel;
    juce::Label mixLabel;
    juce::Label distortionLabel;
    juce::Label volumeLabel;
    juce::Label thresholdLabel;
    juce::Label releaseLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> distortionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrainCosmosAudioProcessorEditor)
};

