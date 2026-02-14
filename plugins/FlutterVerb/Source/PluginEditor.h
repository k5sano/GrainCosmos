#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class FlutterVerbAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    FlutterVerbAudioProcessorEditor(FlutterVerbAudioProcessor& p);
    ~FlutterVerbAudioProcessorEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    FlutterVerbAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlutterVerbAudioProcessorEditor)
};
