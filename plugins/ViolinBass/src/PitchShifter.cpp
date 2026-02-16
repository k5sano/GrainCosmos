#include "PitchShifter.h"
#include <cmath>
#include <cstring>

PitchShifter::PitchShifter() = default;

PitchShifter::~PitchShifter() = default;

void PitchShifter::prepare(double newSampleRate, int samplesPerBlock)
{
    if (newSampleRate != sampleRate || blockSize != samplesPerBlock || stretcher == nullptr)
    {
        sampleRate = newSampleRate;
        blockSize = samplesPerBlock;

        // Initialize input buffer
        inputBuffer.setSize(numChannels, samplesPerBlock);

        // Initialize ring buffer
        outputRingBuffer.setSize(numChannels, RING_BUFFER_SIZE);
        outputRingBuffer.clear();
        writePos = 0;
        readPos = 0;
        bufferedSamples = 0;
        samplesProcessed = 0;

        createStretcher();
    }
}

void PitchShifter::reset()
{
    if (stretcher != nullptr)
    {
        stretcher->reset();
    }
    // Reset ring buffer
    outputRingBuffer.clear();
    writePos = 0;
    readPos = 0;
    bufferedSamples = 0;
    samplesProcessed = 0;
}

void PitchShifter::setPitchSemiTones(double semitones)
{
    double newPitchScale = std::pow(2.0, semitones / 12.0);
    if (std::abs(newPitchScale - pitchScale) < 0.0001)
        return;
    pitchScale = newPitchScale;
    fprintf(stderr, "[setPitchSemiTones] semitones=%.2f, pitchScale=%.3f\n", semitones, pitchScale);
    if (stretcher != nullptr)
        stretcher->setPitchScale(pitchScale);
}

void PitchShifter::setFormantPreserved(bool preserve)
{
    if (formantPreserved == preserve)
        return;
    formantPreserved = preserve;
    createStretcher();
}

void PitchShifter::createStretcher()
{
    // Check sample rate is valid
    if (sampleRate <= 0.0)
    {
        startDelay = 0;
        latencyMs = 0.0;
        return;
    }

    // Rubber Band options for low-latency pitch shifting
    int options = 0;

    // Essential options for violin-to-bass:
    options |= RubberBand::RubberBandStretcher::OptionProcessRealTime;  // Real-time mode
    options |= RubberBand::RubberBandStretcher::OptionEngineFiner;      // R3 Finer engine
    options |= RubberBand::RubberBandStretcher::OptionWindowShort;      // Short window for low latency
    options |= RubberBand::RubberBandStretcher::OptionPitchHighConsistency; // High pitch consistency
    options |= RubberBand::RubberBandStretcher::OptionTransientsSmooth; // Smooth transients for lower latency

    if (formantPreserved)
        options |= RubberBand::RubberBandStretcher::OptionFormantPreserved;
    else
        options |= RubberBand::RubberBandStretcher::OptionFormantShifted;

    // Create the stretcher using C++ API
    stretcher = std::make_unique<RubberBand::RubberBandStretcher>(
        static_cast<size_t>(sampleRate),
        static_cast<size_t>(numChannels),
        options,
        1.0,      // time ratio (1.0 = no stretch)
        pitchScale // initial pitch scale
    );

    // Set max process size to avoid reallocation
    stretcher->setMaxProcessSize(static_cast<size_t>(blockSize));

    // Get latency information
    startDelay = static_cast<int>(stretcher->getLatency());
    latencyMs = (startDelay / sampleRate) * 1000.0;

    // Log latency for debugging
    fprintf(stderr, "Rubber Band C++ initialized:\n");
    fprintf(stderr, "  Sample rate: %.0f\n", sampleRate);
    fprintf(stderr, "  Channels: %d\n", numChannels);
    fprintf(stderr, "  Block size: %d\n", blockSize);
    fprintf(stderr, "  Engine version: %d\n", stretcher->getEngineVersion());
    fprintf(stderr, "  Preferred start pad: %zu\n", stretcher->getPreferredStartPad());
    fprintf(stderr, "  Start delay: %d samples (%.1f ms)\n", startDelay, latencyMs);
    fprintf(stderr, "  Pitch scale: %.3f\n", pitchScale);
    fprintf(stderr, "  Formant preserved: %s\n", formantPreserved ? "yes" : "no");

    // Feed start pad (zero padding) as required by Rubber Band R3 engine
    size_t pad = stretcher->getPreferredStartPad();
    if (pad > 0)
    {
        fprintf(stderr, "  Feeding start pad: %zu zero samples\n", pad);
        juce::AudioBuffer<float> padBuffer(numChannels, static_cast<int>(pad));
        padBuffer.clear();
        std::vector<const float*> padPtrs(numChannels);
        for (int ch = 0; ch < numChannels; ++ch)
            padPtrs[ch] = padBuffer.getReadPointer(ch);
        stretcher->process(padPtrs.data(), pad, false);

        // Try to retrieve after padding
        size_t avail = stretcher->available();
        fprintf(stderr, "  After pad, available: %zu\n", avail);
        if (avail > 0)
        {
            // Put any available samples into ring buffer
            std::vector<float*> outputPtrs(numChannels);
            for (int ch = 0; ch < numChannels; ++ch)
                outputPtrs[ch] = outputRingBuffer.getWritePointer(ch);
            int retrieved = static_cast<int>(stretcher->retrieve(outputPtrs.data(), avail));
            bufferedSamples += retrieved;
            writePos = (writePos + retrieved) % RING_BUFFER_SIZE;
            fprintf(stderr, "  Retrieved from pad: %d samples\n", retrieved);
        }
    }
}

void PitchShifter::process(juce::AudioBuffer<float>& buffer)
{
    if (stretcher == nullptr)
        return;

    const int numSamples = buffer.getNumSamples();
    const int numCh = buffer.getNumChannels();

    // Debug: track process calls
    static int processCallCount = 0;
    if (++processCallCount <= 20 || processCallCount % 100 == 0)
    {
        fprintf(stderr, "[process #%d] numSamples=%d, samplesProcessed=%d, startDelay=%d, bufferedSamples=%d, available=%zu\n",
                processCallCount, numSamples, samplesProcessed, startDelay, bufferedSamples, stretcher->available());
    }

    // 入力をコピー
    inputBuffer.makeCopyOf(buffer);

    const float* inputPtrs[2];
    for (int ch = 0; ch < numCh; ++ch)
        inputPtrs[ch] = inputBuffer.getReadPointer(ch);

    // Rubber Band に入力を渡す（ブロックサイズそのまま）
    stretcher->process(inputPtrs, static_cast<size_t>(numSamples), false);

    // 出力を取れるだけリングバッファに取り込む
    int available = static_cast<int>(stretcher->available());
    while (available > 0)
    {
        int spaceInRing = RING_BUFFER_SIZE - bufferedSamples;
        if (spaceInRing <= 0) break;

        int contiguous = RING_BUFFER_SIZE - writePos;
        int toRetrieve = std::min({available, spaceInRing, contiguous});

        float* outPtrs[2];
        for (int ch = 0; ch < numCh; ++ch)
            outPtrs[ch] = outputRingBuffer.getWritePointer(ch) + writePos;

        size_t retrieved = stretcher->retrieve(outPtrs, static_cast<size_t>(toRetrieve));
        writePos = (writePos + static_cast<int>(retrieved)) % RING_BUFFER_SIZE;
        bufferedSamples += static_cast<int>(retrieved);

        available = static_cast<int>(stretcher->available());
    }

    samplesProcessed += numSamples;

    // Debug output for buffer state
    if (processCallCount <= 20 || processCallCount % 100 == 0)
    {
        fprintf(stderr, "  Output: bufferedSamples=%d, numSamples=%d, will%s\n",
                bufferedSamples, numSamples, (bufferedSamples >= numSamples) ? " output wet" : " clear");
    }

    // リングバッファから出力
    if (bufferedSamples >= numSamples)
    {
        for (int ch = 0; ch < numCh; ++ch)
        {
            auto* dest = buffer.getWritePointer(ch);
            auto* src = outputRingBuffer.getReadPointer(ch);

            int firstChunk = std::min(numSamples, RING_BUFFER_SIZE - readPos);
            std::memcpy(dest, src + readPos, static_cast<size_t>(firstChunk) * sizeof(float));

            if (firstChunk < numSamples)
                std::memcpy(dest + firstChunk, src,
                            static_cast<size_t>(numSamples - firstChunk) * sizeof(float));
        }
        readPos = (readPos + numSamples) % RING_BUFFER_SIZE;
        bufferedSamples -= numSamples;
    }
    else
    {
        // まだ溜まっていない（起動直後のみ）→ 無音
        buffer.clear();
    }
}
