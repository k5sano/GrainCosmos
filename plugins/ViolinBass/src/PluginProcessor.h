#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "PitchShifter.h"

class ViolinBassAudioProcessor : public juce::AudioProcessor
{
public:
    ViolinBassAudioProcessor();
    ~ViolinBassAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "ViolinBass"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Process block bus layout
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override
    {
        // Only support stereo-in, stereo-out
        return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo() &&
               layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
    }

    // Access to pitch shifter for latency display
    const PitchShifter& getPitchShifter() const { return pitchShifter; }

    juce::AudioProcessorValueTreeState parameters;

    // Level meters (public for editor access)
    float currentInputLevel = 0.0f;
    float currentOutputLevel = 0.0f;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Pitch shifter (unique_ptr not needed for latency access)
    PitchShifter pitchShifter;

    // Filters
    juce::dsp::LinkwitzRileyFilter<float> lowPassFilter;
    juce::dsp::LinkwitzRileyFilter<float> highPassFilter;

    // Level metering
    float inputLevelDecay = 0.0f;
    float outputLevelDecay = 0.0f;

    // Track previous parameter values to avoid unnecessary updates
    double previousPitchSemitones = -1000.0;
    bool previousFormantPreserved = false;

    // Helper functions
    void updateFilters();
    float calculateLevel(float sample, float& decay);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ViolinBassAudioProcessor)
};
