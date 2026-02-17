#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class AbyssVerbAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    AbyssVerbAudioProcessorEditor(AbyssVerbAudioProcessor&);
    ~AbyssVerbAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    AbyssVerbAudioProcessor& audioProcessor;

    struct KnobWithLabel
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    // バイオリン入力
    KnobWithLabel piezoKnob, bodyKnob, brightnessKnob;
    // リバーブ
    KnobWithLabel decayKnob, dampHighKnob, dampLowKnob, shimmerKnob, swayKnob;
    // ディレイ
    KnobWithLabel echoTimeKnob, echoSustainKnob, vanishKnob, fadeTexKnob, driftKnob, chorusKnob;
    // ミックス
    KnobWithLabel reverbMixKnob, delayMixKnob, masterMixKnob, bowSensKnob;

    void setupKnob(KnobWithLabel& knob, const juce::String& paramId,
                   const juce::String& labelText,
                   juce::Colour fillColour = juce::Colour(0xFF4A9EBF));

    juce::Random paintRandom;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AbyssVerbAudioProcessorEditor)
};
