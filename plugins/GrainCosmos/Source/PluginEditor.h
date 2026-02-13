#pragma once
#include "PluginProcessor.h"

class GrainCosmosAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit GrainCosmosAudioProcessorEditor(GrainCosmosAudioProcessor&);
    ~GrainCosmosAudioProcessorEditor() override;

    void resized() override;

private:
    GrainCosmosAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrainCosmosAudioProcessorEditor)
};
