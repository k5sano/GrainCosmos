#pragma once
#include "PluginProcessor.h"

class RadioMusicAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit RadioMusicAudioProcessorEditor(RadioMusicAudioProcessor&);
    ~RadioMusicAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    RadioMusicAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RadioMusicAudioProcessorEditor)
};
