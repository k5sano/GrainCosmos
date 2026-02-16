#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

struct OscillatorState
{
    float phase = 0.0f;
    float phaseIncrement = 0.0f;

    // DC filter state (1st order highpass at ~20Hz)
    float dcFilterZ1 = 0.0f;

    // Self-feedback states
    float prevOutput = 0.0f;  // For AM feedback
    static constexpr int maxDelaySamples = 2400;  // 50ms @ 48kHz
    std::array<float, maxDelaySamples> delayBuffer{};
    int delayWritePos = 0;
    float delayReadPos = 0.0f;  // For interpolated readout
};

class DroneCosmosAudioProcessor : public juce::AudioProcessor
{
public:
    DroneCosmosAudioProcessor();
    ~DroneCosmosAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "DroneCosmos"; }
    bool acceptsMidi() const override { return false; }
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

    juce::AudioProcessorValueTreeState parameters;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Oscillator states
    std::array<OscillatorState, 4> oscStates;

    // Chaos LFO for modulation randomness
    float chaosLFOPhase = 0.0f;
    float chaosLFOIncrement = 0.0f;

    double currentSampleRate = 44100.0;

    // Limiter
    juce::dsp::Limiter<float> limiter;

    // Helper functions
    float generateWaveform(float phase, float waveformParam);
    float softClip(float sample);
    float dcFilter(float sample, float& z1, float rc);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DroneCosmosAudioProcessor)
};
