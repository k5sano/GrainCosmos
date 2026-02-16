#pragma once

#include <rubberband/RubberBandStretcher.h>
#include <juce_dsp/juce_dsp.h>
#include <memory>

/**
 * Rubber Band C++ API wrapper for real-time pitch shifting
 * Optimized for low-latency violin-to-bass conversion
 */
class PitchShifter
{
public:
    PitchShifter();
    ~PitchShifter();

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();

    void setPitchSemiTones(double semitones);
    void setFormantPreserved(bool preserve);

    /**
     * Process a block of audio
     * Input and output can be the same buffer (in-place processing)
     */
    void process(juce::AudioBuffer<float>& buffer);

    /**
     * Get the current latency in samples
     */
    int getLatency() const { return startDelay; }

    /**
     * Get the current latency in milliseconds
     */
    double getLatencyMs() const { return latencyMs; }

private:
    std::unique_ptr<RubberBand::RubberBandStretcher> stretcher;
    double sampleRate = 0.0;
    int numChannels = 2;
    int blockSize = 0;

    // Rubber Band state
    double pitchScale = 1.0;       // 1.0 = no change
    bool formantPreserved = true;
    int startDelay = 0;             // samples
    double latencyMs = 0.0;         // milliseconds

    // Input buffer for Rubber Band (non-interleaved)
    juce::AudioBuffer<float> inputBuffer;

    // Ring buffer for smooth output
    juce::AudioBuffer<float> outputRingBuffer;
    int writePos = 0;
    int readPos = 0;
    int bufferedSamples = 0;
    static constexpr int RING_BUFFER_SIZE = 16384;
    int samplesProcessed = 0;

    void createStretcher();
};
