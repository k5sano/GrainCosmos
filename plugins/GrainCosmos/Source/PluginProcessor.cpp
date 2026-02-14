#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout GrainCosmosAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "delay_time", 1 },
        "Delay Time",
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f, 0.5f),
        0.5f,
        "s"
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "grain_size", 1 },
        "Grain Size",
        juce::NormalisableRange<float>(10.0f, 500.0f, 1.0f, 0.4f),
        100.0f,
        "ms"
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "envelope_shape", 1 },
        "Envelope Shape",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f, 1.0f),
        0.5f,
        ""
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "distortion_amount", 1 },
        "Distortion",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        0.0f,
        "%"
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "feedback", 1 },
        "Feedback",
        juce::NormalisableRange<float>(0.0f, 95.0f, 0.1f, 1.0f),
        40.0f,
        "%"
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "feedback_saturation", 1 },
        "Feedback Fuzz",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        0.0f,
        "%"
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "chaos", 1 },
        "Chaos",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        20.0f,
        "%"
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "character", 1 },
        "Character",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        50.0f,
        "%"
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "grain_voices", 1 },
        "Grain Voices",
        juce::NormalisableRange<float>(1.0f, 64.0f, 1.0f),
        32.0f,
        ""
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "mix", 1 },
        "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        30.0f,
        "%"
    ));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "freeze", 1 },
        "Freeze",
        false
    ));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "tempo_sync", 1 },
        "Tempo Sync",
        false
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "output_volume", 1 },
        "Output Volume",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f),
        100.0f,
        "%"
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "limiter_threshold", 1 },
        "Limiter Threshold",
        juce::NormalisableRange<float>(-20.0f, 0.0f, 0.1f),
        -1.0f,
        "dB"
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "limiter_release", 1 },
        "Limiter Release",
        juce::NormalisableRange<float>(10.0f, 500.0f, 1.0f),
        100.0f,
        "ms"
    ));

    return layout;
}

//==============================================================================
GrainCosmosAudioProcessor::GrainCosmosAudioProcessor()
    : AudioProcessor(BusesProperties()
                        .withInput("Input", juce::AudioChannelSet::stereo(), true)
                        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , parameters(*this, nullptr, "Parameters", createParameterLayout())
{
}

GrainCosmosAudioProcessor::~GrainCosmosAudioProcessor()
{
}

//==============================================================================
void GrainCosmosAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 2;

    int maxDelaySamples = static_cast<int>(sampleRate * maxDelaySeconds);
    grainBuffer.setMaximumDelayInSamples(maxDelaySamples);
    grainBuffer.prepare(spec);
    grainBuffer.reset();

    for (auto& voice : grainVoices)
    {
        voice.active = false;
        voice.readPosition = 0.0f;
        voice.windowPosition = 0.0f;
        voice.playbackRate = 1.0f;
        voice.pan = 0.5f;
        voice.grainLengthSamples = 0;
        voice.pitchSemitones = 0;
    }

    samplesSinceLastGrain = 0;
    writePosition = 0;
    feedbackSampleL = 0.0f;
    feedbackSampleR = 0.0f;

    freezeEnabled.store(false, std::memory_order_relaxed);
    inputGainRamp = 1.0f;
    samplesSinceUnfreeze = unfreezeRampSamples;

    auto delayTimeParam = parameters.getRawParameterValue("delay_time");
    float delayTimeSec = delayTimeParam->load();
    nextGrainInterval = static_cast<int>(delayTimeSec * sampleRate);

    wetBuffer.setSize(2, samplesPerBlock);
    dryBuffer.setSize(2, samplesPerBlock);

    limiter.prepare(spec);
    limiter.reset();
}

void GrainCosmosAudioProcessor::releaseResources()
{
    wetBuffer.setSize(0, 0);
    dryBuffer.setSize(0, 0);
}

void GrainCosmosAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    const int numSamples = buffer.getNumSamples();

    if (wetBuffer.getNumSamples() < numSamples)
    {
        wetBuffer.setSize(2, numSamples, false, false, true);
        dryBuffer.setSize(2, numSamples, false, false, true);
    }

    // Read parameters
    auto delayTimeParam = parameters.getRawParameterValue("delay_time");
    auto mixParam = parameters.getRawParameterValue("mix");
    auto feedbackParam = parameters.getRawParameterValue("feedback");
    auto feedbackSatParam = parameters.getRawParameterValue("feedback_saturation");
    auto characterParam = parameters.getRawParameterValue("character");
    auto chaosParam = parameters.getRawParameterValue("chaos");
    auto grainSizeParam = parameters.getRawParameterValue("grain_size");
    auto envelopeShapeParam = parameters.getRawParameterValue("envelope_shape");
    auto distortionParam = parameters.getRawParameterValue("distortion_amount");
    auto freezeParam = parameters.getRawParameterValue("freeze");
    auto tempoSyncParam = parameters.getRawParameterValue("tempo_sync");
    auto grainVoicesParam = parameters.getRawParameterValue("grain_voices");

    float delayTimeSec = delayTimeParam->load();
    float mixValue = mixParam->load() / 100.0f;
    float feedbackGain = (feedbackParam->load() / 100.0f) * 0.95f;
    float feedbackSaturation = feedbackSatParam->load();
    float characterAmount = characterParam->load() / 100.0f;
    float chaosAmount = chaosParam->load() / 100.0f;
    float grainSizeMs = grainSizeParam->load();
    float envelopeShape = envelopeShapeParam->load();
    float distortionAmount = distortionParam->load();
    bool freezeRequested = freezeParam->load() > 0.5f;
    bool tempoSyncEnabled = tempoSyncParam->load() > 0.5f;

    // Update voice limit
    currentVoiceLimit = static_cast<int>(grainVoicesParam->load());
    currentVoiceLimit = juce::jlimit(1, maxGrainVoices, currentVoiceLimit);

    // Character morphing
    float densityMultiplier = 1.0f + (characterAmount * 3.0f);
    float tukeyAlpha = 0.1f + (characterAmount * 0.9f);

    // Tempo sync
    float delayTimeSecQuantized = getQuantizedDelayTime(delayTimeSec, tempoSyncEnabled);

    // Freeze state handling
    if (freezeRequested != freezeEnabled.load(std::memory_order_relaxed))
    {
        freezeEnabled.store(freezeRequested, std::memory_order_relaxed);
        if (!freezeRequested)
        {
            inputGainRamp = 0.0f;
            samplesSinceUnfreeze = 0;
        }
    }

    if (!freezeEnabled.load() && inputGainRamp < 1.0f)
    {
        samplesSinceUnfreeze += numSamples;
        inputGainRamp = static_cast<float>(samplesSinceUnfreeze) / static_cast<float>(unfreezeRampSamples);
        if (inputGainRamp > 1.0f)
            inputGainRamp = 1.0f;
    }
    else if (freezeEnabled.load())
    {
        inputGainRamp = 0.0f;
    }
    else
    {
        inputGainRamp = 1.0f;
    }

    // Grain spawn interval
    float baseIntervalSamples = delayTimeSecQuantized * static_cast<float>(currentSampleRate);
    nextGrainInterval = static_cast<int>(baseIntervalSamples / densityMultiplier);
    if (nextGrainInterval < 1)
        nextGrainInterval = 1;

    const float* inputL = buffer.getReadPointer(0);
    const float* inputR = buffer.getNumChannels() > 1 ? buffer.getReadPointer(1) : buffer.getReadPointer(0);

    wetBuffer.clear();

    for (int i = 0; i < numSamples; ++i)
    {
        dryBuffer.setSample(0, i, inputL[i]);
        dryBuffer.setSample(1, i, inputR[i]);
    }

    // Sample-by-sample processing
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Freeze: input gain ramp applies to input only, NOT feedback
        float inputOnlyL = inputL[sample] * inputGainRamp;
        float inputOnlyR = inputR[sample] * inputGainRamp;
        float inputWithFeedbackL = inputOnlyL + feedbackSampleL;
        float inputWithFeedbackR = inputOnlyR + feedbackSampleR;

        // Write to buffer only if not frozen
        if (!freezeEnabled.load(std::memory_order_relaxed))
        {
            grainBuffer.pushSample(0, inputWithFeedbackL);
            grainBuffer.pushSample(1, inputWithFeedbackR);
        }

        // Grain scheduling with chaos jitter
        int currentInterval = nextGrainInterval;
        if (chaosAmount > 0.01f)
        {
            float timingJitter = (random.nextFloat() - 0.5f) * chaosAmount;
            currentInterval = static_cast<int>(nextGrainInterval * (1.0f + timingJitter));
            currentInterval = std::max(1, currentInterval);
        }

        samplesSinceLastGrain++;
        if (samplesSinceLastGrain >= currentInterval && currentInterval > 0)
        {
            spawnGrain();
            samplesSinceLastGrain = 0;
        }

        // Process grain voices
        float leftOutput = 0.0f;
        float rightOutput = 0.0f;

        for (auto& voice : grainVoices)
        {
            if (!voice.active)
                continue;

            float delaySamples = std::max(0.0f, voice.readPosition);
            float grainSampleL = grainBuffer.popSample(0, delaySamples, false);
            float grainSampleR = grainBuffer.popSample(1, delaySamples, false);

            // Dual envelope: ADSR x Tukey
            float adsrEnvelope = getADSREnvelope(voice.windowPosition, envelopeShape);
            float tukeyWindow = getWindowSample(voice.windowPosition, tukeyAlpha);
            float combinedEnvelope = adsrEnvelope * tukeyWindow;

            float processedL = grainSampleL * combinedEnvelope;
            float processedR = grainSampleR * combinedEnvelope;

            // Equal-power pan
            float leftGain = std::cos(voice.pan * juce::MathConstants<float>::halfPi);
            float rightGain = std::sin(voice.pan * juce::MathConstants<float>::halfPi);

            leftOutput += (processedL * leftGain + processedR * (1.0f - rightGain)) * 0.707f;
            rightOutput += (processedR * rightGain + processedL * (1.0f - leftGain)) * 0.707f;

            voice.readPosition -= voice.playbackRate;

            float windowIncrement = 1.0f / static_cast<float>(voice.grainLengthSamples);
            voice.windowPosition += windowIncrement;

            if (voice.windowPosition >= 1.0f || voice.readPosition < 0.0f)
            {
                voice.active = false;
            }
        }

        // Feedback path with Fuzz (feedback_saturation)
        feedbackSampleL = leftOutput * feedbackGain;
        feedbackSampleR = rightOutput * feedbackGain;

        if (feedbackSaturation > 0.001f)
        {
            feedbackSampleL = applyDistortion(feedbackSampleL, feedbackSaturation);
            feedbackSampleR = applyDistortion(feedbackSampleR, feedbackSaturation);
        }

        // Soft saturation safety at high feedback
        if (feedbackGain > 0.5f)
        {
            feedbackSampleL = std::tanh(feedbackSampleL);
            feedbackSampleR = std::tanh(feedbackSampleR);
        }

        // Write to wet buffer
        wetBuffer.setSample(0, sample, leftOutput);
        wetBuffer.setSample(1, sample, rightOutput);
    }

    // Dry/wet mix
    float dryGain = 1.0f - mixValue;
    float wetGain = mixValue;

    for (int i = 0; i < numSamples; ++i)
    {
        float dryL = dryBuffer.getSample(0, i);
        float dryR = dryBuffer.getSample(1, i);
        float wetL = wetBuffer.getSample(0, i);
        float wetR = wetBuffer.getSample(1, i);

        float mixedL = dryL * dryGain + wetL * wetGain;
        float mixedR = dryR * dryGain + wetR * wetGain;

        // Final distortion (distortion_amount) - applies to both dry and wet
        if (distortionAmount > 0.001f)
        {
            mixedL = applyDistortion(mixedL, distortionAmount);
            mixedR = applyDistortion(mixedR, distortionAmount);
        }

        buffer.setSample(0, i, mixedL);
        buffer.setSample(1, i, mixedR);
    }

    // Output volume
    auto outputVolParam = parameters.getRawParameterValue("output_volume");
    float outputGain = outputVolParam->load() / 100.0f;
    buffer.applyGain(outputGain);

    // Limiter
    auto limThreshParam = parameters.getRawParameterValue("limiter_threshold");
    auto limReleaseParam = parameters.getRawParameterValue("limiter_release");
    float thresholdDb = limThreshParam->load();
    float releaseMs = limReleaseParam->load();

    limiter.setThreshold(thresholdDb);
    limiter.setRelease(releaseMs);

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    limiter.process(context);
}

//==============================================================================
juce::AudioProcessorEditor* GrainCosmosAudioProcessor::createEditor()
{
    return new GrainCosmosAudioProcessorEditor(*this);
}

//==============================================================================
void GrainCosmosAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void GrainCosmosAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GrainCosmosAudioProcessor();
}

//==============================================================================
void GrainCosmosAudioProcessor::spawnGrain()
{
    int voiceIndex = findFreeVoice();
    if (voiceIndex < 0)
        return;

    auto& voice = grainVoices[static_cast<size_t>(voiceIndex)];

    auto grainSizeParam = parameters.getRawParameterValue("grain_size");
    auto delayTimeParam = parameters.getRawParameterValue("delay_time");
    auto chaosParam = parameters.getRawParameterValue("chaos");
    auto tempoSyncParam = parameters.getRawParameterValue("tempo_sync");

    float grainSizeMs = grainSizeParam->load();
    float delayTimeSec = delayTimeParam->load();
    float chaosAmount = chaosParam->load() / 100.0f;
    bool tempoSyncEnabled = tempoSyncParam->load() > 0.5f;

    float delayTimeSecQuantized = getQuantizedDelayTime(delayTimeSec, tempoSyncEnabled);

    voice.grainLengthSamples = static_cast<int>((grainSizeMs / 1000.0f) * currentSampleRate);
    if (voice.grainLengthSamples < 1)
        voice.grainLengthSamples = 1;

    float delayTimeSamples = delayTimeSecQuantized * static_cast<float>(currentSampleRate);

    float positionJitter = (random.nextFloat() - 0.5f) * chaosAmount * 0.5f;
    float basePosition = delayTimeSamples;
    voice.readPosition = basePosition * (1.0f + positionJitter);

    float maxDelaySamples = static_cast<float>(currentSampleRate * maxDelaySeconds);
    voice.readPosition = juce::jlimit(1.0f, maxDelaySamples - 1.0f, voice.readPosition);

    voice.windowPosition = 0.0f;

    voice.pitchSemitones = selectPitchShift(chaosAmount);
    voice.playbackRate = calculatePlaybackRate(voice.pitchSemitones);

    float panRandomness = (random.nextFloat() - 0.5f) * 2.0f;
    voice.pan = 0.5f + (panRandomness * 0.5f * chaosAmount);
    voice.pan = juce::jlimit(0.0f, 1.0f, voice.pan);

    voice.active = true;
}

int GrainCosmosAudioProcessor::findFreeVoice()
{
    for (int i = 0; i < currentVoiceLimit; ++i)
    {
        if (!grainVoices[static_cast<size_t>(i)].active)
            return i;
    }
    return -1;
}

int GrainCosmosAudioProcessor::selectPitchShift(float chaosAmount)
{
    if (chaosAmount < 0.01f)
        return 0;

    float randomValue = random.nextFloat();
    float threshold = chaosAmount;
    if (randomValue > threshold)
        return 0;

    const int pitchOptions[] = { -12, -7, 7, 12 };
    int index = static_cast<int>(random.nextFloat() * 4.0f);
    index = juce::jlimit(0, 3, index);
    return pitchOptions[index];
}

float GrainCosmosAudioProcessor::calculatePlaybackRate(int semitones)
{
    return std::pow(2.0f, static_cast<float>(semitones) / 12.0f);
}

float GrainCosmosAudioProcessor::getWindowSample(float normalizedPosition, float tukeyAlpha)
{
    if (normalizedPosition < 0.0f)
        normalizedPosition = 0.0f;
    if (normalizedPosition >= 1.0f)
        normalizedPosition = 0.9999f;

    float x = normalizedPosition;
    float windowValue;

    if (x < tukeyAlpha / 2.0f)
    {
        windowValue = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * x / tukeyAlpha));
    }
    else if (x < 1.0f - tukeyAlpha / 2.0f)
    {
        windowValue = 1.0f;
    }
    else
    {
        windowValue = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * (1.0f - x) / tukeyAlpha));
    }

    return windowValue;
}

//==============================================================================
float GrainCosmosAudioProcessor::getADSREnvelope(float windowPos, float envelopeShape)
{
    if (windowPos < 0.0f)
        windowPos = 0.0f;
    if (windowPos >= 1.0f)
        windowPos = 0.9999f;

    // Percussive: fast attack, fast decay
    float envPercussive;
    if (windowPos < 0.05f)
        envPercussive = windowPos / 0.05f;
    else
        envPercussive = 1.0f - (windowPos - 0.05f) / 0.95f;

    // Balanced: medium attack, medium decay, low sustain
    float envBalanced;
    if (windowPos < 0.15f)
        envBalanced = windowPos / 0.15f;
    else if (windowPos < 0.65f)
        envBalanced = 1.0f - (windowPos - 0.15f) * 0.5f;
    else
        envBalanced = (1.0f - windowPos) / 0.35f;

    // Smooth: slow attack, long sustain, slow release
    float envSmooth;
    if (windowPos < 0.30f)
        envSmooth = windowPos / 0.30f;
    else if (windowPos < 0.50f)
        envSmooth = 1.0f - (windowPos - 0.30f) * 0.5f;
    else if (windowPos < 0.90f)
        envSmooth = 1.0f;
    else
        envSmooth = (1.0f - windowPos) / 0.10f;

    // Crossfade between shapes
    float adsrEnvelope;
    if (envelopeShape < 0.5f)
    {
        float t = envelopeShape * 2.0f;
        adsrEnvelope = envPercussive * (1.0f - t) + envBalanced * t;
    }
    else
    {
        float t = (envelopeShape - 0.5f) * 2.0f;
        adsrEnvelope = envBalanced * (1.0f - t) + envSmooth * t;
    }

    return adsrEnvelope;
}

//==============================================================================
float GrainCosmosAudioProcessor::applyDistortion(float sample, float distortionAmount)
{
    float normDistortion = distortionAmount / 100.0f;
    if (normDistortion < 0.001f)
        return sample;

    // Gentler curve: max 7.5x (half of 15x)
    float drive = 1.0f + normDistortion * 7.5f;
    float x = sample * drive;

    float biasOffset = normDistortion * 0.15f;  // Half of 0.3f
    x += biasOffset;

    if (x > 1.0f)
        x = 1.0f - (x - 1.0f) * 0.5f;
    if (x < -0.7f)
        x = -0.7f + (x + 0.7f) * 0.3f;

    // Final tanh: gentler curve
    return std::tanh(x * (1.0f + normDistortion * 0.5f));
}

//==============================================================================
float GrainCosmosAudioProcessor::getQuantizedDelayTime(float delayTimeSec, bool tempoSync)
{
    if (!tempoSync)
        return delayTimeSec;

    double bpm = 120.0;
    if (auto* playHead = getPlayHead())
    {
        auto currentPosition = playHead->getPosition();
        if (currentPosition.hasValue())
        {
            auto bpmValue = currentPosition->getBpm();
            if (bpmValue.hasValue())
                bpm = *bpmValue;
        }
    }

    bpm = juce::jlimit(20.0, 300.0, bpm);

    float sixteenthTime = 60.0f / static_cast<float>(bpm * 4.0);
    float eighthTime = 60.0f / static_cast<float>(bpm * 2.0);
    float quarterTime = 60.0f / static_cast<float>(bpm);
    float halfTime = 60.0f * 2.0f / static_cast<float>(bpm);
    float wholeTime = 60.0f * 4.0f / static_cast<float>(bpm);

    const float noteDivisions[] = { sixteenthTime, eighthTime, quarterTime, halfTime, wholeTime };
    const int numDivisions = 5;

    float nearestDivision = delayTimeSec;
    float minDistance = std::abs(delayTimeSec - noteDivisions[0]);

    for (int i = 0; i < numDivisions; ++i)
    {
        float distance = std::abs(delayTimeSec - noteDivisions[i]);
        if (distance < minDistance)
        {
            minDistance = distance;
            nearestDivision = noteDivisions[i];
        }
    }

    if (nearestDivision > 2.0f)
        nearestDivision = 2.0f;

    return nearestDivision;
}
