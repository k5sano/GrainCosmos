#include "PluginProcessor.h"
#include "PluginEditor.h"

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

    // Setup DSP spec for stereo
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 2;  // Stereo input/output

    // Prepare the delay buffer for maximum delay time
    // Set maximum delay size BEFORE preparing with spec
    int maxDelaySamples = static_cast<int>(sampleRate * maxDelaySeconds);
    grainBuffer.setMaximumDelayInSamples(maxDelaySamples);
    grainBuffer.prepare(spec);

    // Reset freeze state on prepare
    freezeEnabled.store(false, std::memory_order_relaxed);
    inputGainRamp = 1.0f;
    samplesSinceUnfreeze = 0;
}

//==============================================================================
void GrainCosmosAudioProcessor::releaseResources()
{
    // Optional: Release large buffers to save memory when plugin not in use
    wetBuffer.setSize(0, 0);
    dryBuffer.setSize(0, 0);
}

void GrainCosmosAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    const int numSamples = buffer.getNumSamples();

    // Ensure buffers are large enough (in case host uses different block size)
    if (wetBuffer.getNumSamples() < numSamples)
    {
        wetBuffer.setSize(2, numSamples, false, false, true);
        dryBuffer.setSize(2, numSamples, false, false, true);
    }

    // Read parameters atomically (real-time safe)
    auto delayTimeParam = parameters.getRawParameterValue("delay_time");
    auto mixParam = parameters.getRawParameterValue("mix");
    auto feedbackParam = parameters.getRawParameterValue("feedback");
    auto feedbackSatParam = parameters.getRawParameterValue("feedback_saturation");
    auto grainDensityParam = parameters.getRawParameterValue("grain_density");
    auto pitchRangeParam = parameters.getRawParameterValue("pitch_range");
    auto bufferPosParam = parameters.getRawParameterValue("buffer_position");
    auto reverseProbParam = parameters.getRawParameterValue("reverse_probability");
    auto stereoSpreadParam = parameters.getRawParameterValue("stereo_spread");
    auto attackTimeParam = parameters.getRawParameterValue("attack_time");
    auto decayTimeParam = parameters.getRawParameterValue("decay_time");
    auto shimmerParam = parameters.getRawParameterValue("shimmer");
    auto lofiParam = parameters.getRawParameterValue("lofi");
    auto characterParam = parameters.getRawParameterValue("character");
    auto chaosParam = parameters.getRawParameterValue("chaos");
    auto grainSizeParam = parameters.getRawParameterValue("grain_size");
    auto envelopeShapeParam = parameters.getRawParameterValue("envelope_shape");
    auto distortionParam = parameters.getRawParameterValue("distortion_amount");
    auto freezeParam = parameters.getRawParameterValue("freeze");
    auto tempoSyncParam = parameters.getRawParameterValue("tempo_sync");

    float delayTimeSec = delayTimeParam->load();
    float mixValue = mixParam->load() / 100.0f;
    float feedbackGain = feedbackParam->load() / 100.0f;
    float feedbackSaturation = feedbackSatParam->load();  // Fuzz in feedback loop
    float grainDensity = grainDensityParam->load();  // 1.0 - 32.0 voices
    float pitchRange = pitchRangeParam->load() / 100.0f;  // 0.0 - 1.0 (±24 semitones)
    float bufferPosition = bufferPosParam->load() / 100.0f;  // 0.0 - 1.0
    float reverseProbability = reverseProbParam->load() / 100.0f;  // Reverse grain probability
    float stereoSpread = stereoSpreadParam->load() / 100.0f;  // Stereo width
    float attackTime = attackTimeParam->load() / 100.0f;  // Envelope attack
    float decayTime = decayTimeParam->load() / 100.0f;  // Envelope decay
    float shimmerAmount = shimmerParam->load() / 100.0f;  // Pitch-shift feedback
    float lofiAmount = lofiParam->load() / 100.0f;  // Bitcrush + rate reduction
    float characterAmount = characterParam->load() / 100.0f;
    float chaosAmount = chaosParam->load() / 100.0f;
    float grainSizeMs = grainSizeParam->load();
    float envelopeShape = envelopeShapeParam->load();      // Phase 4.2: ADSR shape
    float distortionAmount = distortionParam->load();    // Phase 4.3: 0-100%
    bool freezeRequested = freezeParam->load() > 0.5f;      // Phase 4.3: Freeze state
    bool tempoSyncEnabled = tempoSyncParam->load() > 0.5f; // Phase 4.4: Tempo sync

    // Character morphing: density multiplier (1.0 to 4.0)
    float densityMultiplier = 1.0f + (characterAmount * 3.0f);

    // Calculate Tukey window alpha for character control (0.1 to 1.0)
    float tukeyAlpha = 0.1f + (characterAmount * 0.9f);

    // Phase 4.4: Tempo sync - quantize delay_time to note divisions
    float delayTimeSecQuantized = getQuantizedDelayTime(delayTimeSec, tempoSyncEnabled);

    // Phase 4.3: Handle freeze state with input gain ramp
    if (freezeRequested != freezeEnabled.load(std::memory_order_relaxed))
    {
        // Freeze state changed - update atomic flag
        freezeEnabled.store(freezeRequested, std::memory_order_relaxed);

        if (!freezeRequested)
        {
            // Unfreeze: start input gain ramp from 0.0 to 1.0 over 50ms
            inputGainRamp = 0.0f;
            samplesSinceUnfreeze = 0;
        }
    }

    // Process input gain ramp for smooth unfreeze transition (Phase 4.3)
    if (!freezeEnabled.load() && inputGainRamp < 1.0f)
    {
        samplesSinceUnfreeze += numSamples;
        inputGainRamp = static_cast<float>(samplesSinceUnfreeze) / static_cast<float>(unfreezeRampSamples);
        if (inputGainRamp > 1.0f)
            inputGainRamp = 1.0f;
    }
    else if (freezeEnabled.load())
    {
        inputGainRamp = 0.0f;  // No input gain when frozen
    }
    else
    {
        inputGainRamp = 1.0f;  // Normal operation
    }

    // Recalculate spawn interval with tempo-synced delay time (Phase 4.4)
    float baseIntervalSamples = delayTimeSecQuantized * static_cast<float>(currentSampleRate);
    nextGrainInterval = static_cast<int>(baseIntervalSamples / densityMultiplier);
    if (nextGrainInterval < 1)
        nextGrainInterval = 1;

    // Get stereo input pointers
    const float* inputL = buffer.getReadPointer(0);
    const float* inputR = buffer.getNumChannels() > 1 ? buffer.getReadPointer(1) : buffer.getReadPointer(0);

    // Clear wet buffer
    wetBuffer.clear();

    // Store dry signal for later mixing (preserve stereo field)
    for (int i = 0; i < numSamples; ++i)
    {
        dryBuffer.setSample(0, i, inputL[i]);
        dryBuffer.setSample(1, i, inputR[i]);
    }

    // Process sample by sample
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Phase 4.3: Apply input gain ramp to input only (NOT feedback)
        // This allows feedback loop to continue during freeze
        float inputOnlyL = inputL[sample] * inputGainRamp;
        float inputOnlyR = inputR[sample] * inputGainRamp;

        // Mix input with feedback (feedback is NOT affected by inputGainRamp)
        float inputWithFeedbackL = inputOnlyL + feedbackSampleL;
        float inputWithFeedbackR = inputOnlyR + feedbackSampleR;

        // Phase 4.3: Write to grain buffer only if NOT frozen
        // Freeze stops new input, but grains continue reading existing buffer
        if (!freezeEnabled.load(std::memory_order_relaxed))
        {
            grainBuffer.pushSample(0, inputWithFeedbackL);
            grainBuffer.pushSample(1, inputWithFeedbackR);
        }
        // If frozen, buffer holds current content (grains continue reading)

        // Calculate grain interval with chaos timing jitter
        int currentInterval = nextGrainInterval;
        if (chaosAmount > 0.001f)
        {
            float timingJitter = (random.nextFloat() - 0.5f) * chaosAmount;
            currentInterval = static_cast<int>(nextGrainInterval * (1.0f + timingJitter));
            currentInterval = std::max(1, currentInterval);
        }

        // Check if we should spawn a new grain
        samplesSinceLastGrain++;
        if (samplesSinceLastGrain >= currentInterval && currentInterval > 0)
        {
            spawnGrain();
            samplesSinceLastGrain = 0;
        }

        // Process all active grain voices
        float leftOutput = 0.0f;
        float rightOutput = 0.0f;

        for (auto& voice : grainVoices)
        {
            if (!voice.active)
                continue;

            // Apply buffer position offset (manual read position control)
            float bufferOffsetSamples = bufferPosition * static_cast<float>(grainBuffer.getMaximumDelayInSamples());
            float adjustedReadPos = voice.readPosition + bufferOffsetSamples;

            // Read from delay buffer with interpolation (stereo)
            float delaySamples = std::max(0.0f, adjustedReadPos);
            float grainSampleL = grainBuffer.popSample(0, delaySamples, false);
            float grainSampleR = grainBuffer.popSample(1, delaySamples, false);

            // Phase 4.2: Generate dual envelope (ADSR × Tukey)
            float adsrEnvelope = getADSREnvelope(voice.windowPosition, envelopeShape);
            float tukeyWindow = getWindowSample(voice.windowPosition, tukeyAlpha);
            float combinedEnvelope = adsrEnvelope * tukeyWindow;

            // Apply combined envelope to grain samples
            float processedL = grainSampleL * combinedEnvelope;
            float processedR = grainSampleR * combinedEnvelope;

            // NOTE: Distortion removed from grain voice, now in feedback loop
            // This allows cleaner grains with fuzz accumulating in loop

            // Apply equal-power pan crossfade between stereo channels
            // Pan 0.0 = full left channel, 0.5 = balanced, 1.0 = full right channel
            float leftGain = std::cos(voice.pan * juce::MathConstants<float>::halfPi);
            float rightGain = std::sin(voice.pan * juce::MathConstants<float>::halfPi);

            // Crossfade: at pan=0.5, both channels contribute equally
            // This preserves stereo field while allowing pan randomization
            leftOutput += (processedL * leftGain + processedR * (1.0f - rightGain)) * 0.707f;
            rightOutput += (processedR * rightGain + processedL * (1.0f - leftGain)) * 0.707f;

            // Advance grain playback with pitch randomization
            // pitchRange: 0.0 = unison only, 1.0 = ±24 semitones
            float pitchMod = 1.0f;
            if (pitchRange > 0.001f)
            {
                float semitones = (random.nextFloat() - 0.5f) * 2.0f * pitchRange * 24.0f;
                pitchMod = std::pow(2.0f, semitones / 12.0f);
            }
            voice.readPosition -= voice.playbackRate * pitchMod;

            // Advance window position
            float windowIncrement = 1.0f / static_cast<float>(voice.grainLengthSamples);
            voice.windowPosition += windowIncrement;

            // Check if grain has finished (window complete or read position invalid)
            if (voice.windowPosition >= 1.0f || voice.readPosition < 0.0f)
            {
                voice.active = false;
            }
        }

        // Apply FUZZ distortion to grain output (for direct sound character)
        // feedback_saturation affects both direct sound AND feedback loop
        float distortedL = leftOutput;
        float distortedR = rightOutput;
        if (feedbackSaturation > 0.001f)
        {
            distortedL = applyDistortion(leftOutput, feedbackSaturation);
            distortedR = applyDistortion(rightOutput, feedbackSaturation);
        }

        // Apply feedback gain (after distortion for accumulating chaos)
        feedbackSampleL = distortedL * feedbackGain;
        feedbackSampleR = distortedR * feedbackGain;

        // Write to wet buffer
        wetBuffer.setSample(0, sample, distortedL);
        wetBuffer.setSample(1, sample, distortedR);
    }

    // Linear dry/wet mix (full dry + scaled wet for 0-100%)
    // At 0%: dry only, At 100%: wet only, At 50%: full dry + full wet
    float dryGain = 1.0f - mixValue;  // 1.0 at 0%, 0.0 at 100%
    float wetGain = mixValue;          // 0.0 at 0%, 1.0 at 100%

    for (int i = 0; i < numSamples; ++i)
    {
        float dryL = dryBuffer.getSample(0, i);
        float dryR = dryBuffer.getSample(1, i);
        float wetL = wetBuffer.getSample(0, i);
        float wetR = wetBuffer.getSample(1, i);

        buffer.setSample(0, i, dryL * dryGain + wetL * wetGain);
        buffer.setSample(1, i, dryR * dryGain + wetR * wetGain);
    }
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
// Helper function implementations
void GrainCosmosAudioProcessor::spawnGrain()
{
    int voiceIndex = findFreeVoice();
    if (voiceIndex < 0)
        return;  // No free voices

    auto& voice = grainVoices[static_cast<size_t>(voiceIndex)];
    voice.active = true;
    voice.readPosition = static_cast<float>(writePosition);
    voice.windowPosition = 0.0f;
    voice.pan = random.nextFloat();

    // Default grain size (50ms)
    voice.grainLengthSamples = static_cast<int>((50.0f / 1000.0f) * currentSampleRate);
    if (voice.grainLengthSamples < 1)
        voice.grainLengthSamples = 1;

    // Set playback rate (1.0 = normal speed)
    voice.playbackRate = 1.0f;
}

float GrainCosmosAudioProcessor::getWindowSample(float normalizedPosition, float tukeyAlpha)
{
    // Tukey window: flat middle with tapered edges
    if (normalizedPosition <= 0.0f)
        return 0.0f;
    if (normalizedPosition >= 1.0f)
        return 0.0f;

    float halfAlpha = tukeyAlpha / 2.0f;

    if (normalizedPosition < halfAlpha)
    {
        // Rising cosine edge (left)
        return 0.5f * (1.0f - std::cos(juce::MathConstants<float>::twoPi * normalizedPosition / tukeyAlpha));
    }
    else if (normalizedPosition > 1.0f - halfAlpha)
    {
        // Falling cosine edge (right)
        float pos = 1.0f - normalizedPosition;
        return 0.5f * (1.0f - std::cos(juce::MathConstants<float>::twoPi * pos / tukeyAlpha));
    }
    else
    {
        // Flat middle section
        return 1.0f;
    }
}

int GrainCosmosAudioProcessor::findFreeVoice()
{
    for (int i = 0; i < maxGrainVoices; ++i)
    {
        if (!grainVoices[static_cast<size_t>(i)].active)
            return i;
    }
    return -1;  // No free voices
}

int GrainCosmosAudioProcessor::selectPitchShift(float chaosAmount)
{
    // Simple pitch shift based on chaos amount
    return 0;  // Implement pitch shift logic here
}

float GrainCosmosAudioProcessor::calculatePlaybackRate(int semitones)
{
    return std::pow(2.0f, static_cast<float>(semitones) / 12.0f);
}

float GrainCosmosAudioProcessor::getADSREnvelope(float windowPos, float envelopeShape)
{
    // envelopeShape: 0.0 = attack, 0.5 = both, 1.0 = decay
    if (windowPos <= 0.0f)
        return 0.0f;
    if (windowPos >= 1.0f)
        return 0.0f;

    float attack, decay;
    if (envelopeShape < 0.5f)
    {
        // Attack-focused
        float normalized = envelopeShape * 2.0f;  // 0.0 to 1.0
        attack = 0.1f + normalized * 0.4f;  // 0.1 to 0.5
        decay = 1.0f - attack;
    }
    else
    {
        // Decay-focused
        float normalized = (envelopeShape - 0.5f) * 2.0f;  // 0.0 to 1.0
        decay = 0.1f + normalized * 0.4f;  // 0.1 to 0.5
        attack = 1.0f - decay;
    }

    float value = 1.0f;
    if (windowPos < attack)
    {
        value = windowPos / attack;  // Linear attack
    }
    else if (windowPos > 1.0f - decay)
    {
        value = (1.0f - windowPos) / decay;  // Linear decay
    }

    return value;
}

float GrainCosmosAudioProcessor::applyDistortion(float sample, float distortionAmount)
{
    // Simple fuzz distortion using polynomial waveshaping
    if (distortionAmount < 0.001f)
        return sample;

    // Normalize amount to 0-1
    float amount = distortionAmount / 100.0f;
    float drive = 1.0f + amount * 20.0f;  // 1x to 21x drive

    // Apply soft clip distortion
    float driven = sample * drive;
    float absDriven = std::abs(driven);

    if (absDriven < 1.0f)
    {
        return driven;
    }
    else
    {
        // Soft clipping polynomial
        float sign = (driven > 0.0f) ? 1.0f : -1.0f;
        float soft = sign * (1.0f - std::exp(-absDriven + 1.0f));
        return soft / drive;  // Normalize output
    }
}

float GrainCosmosAudioProcessor::getQuantizedDelayTime(float delayTimeSec, bool tempoSync)
{
    if (!tempoSync)
        return delayTimeSec;

    // Get BPM from host if available
    float bpm = 120.0f;
    if (auto* playHead = getPlayHead())
    {
        auto position = playHead->getPosition();
        if (position.hasValue())
        {
            auto bpmValue = position->getBpm();
            if (bpmValue.hasValue())
                bpm = static_cast<float>(*bpmValue);
        }
    }

    // Note divisions in seconds
    const float divisions[] = {
        4.0f / bpm,      // 16th note
        3.0f / bpm,      // 8th note triplet
        2.0f / bpm,      // 8th note
        1.5f / bpm,      // 8th note triplet
        1.0f / bpm,      // quarter note
        0.75f / bpm,     // quarter note triplet
        0.5f / bpm,      // half note
        0.25f / bpm       // whole note
    };

    // Find closest division
    float closest = delayTimeSec;
    float minDiff = std::abs(delayTimeSec - divisions[0]);
    for (float division : divisions)
    {
        float diff = std::abs(delayTimeSec - division);
        if (diff < minDiff)
        {
            minDiff = diff;
            closest = division;
        }
    }

    return closest;
}

juce::AudioProcessorValueTreeState::ParameterLayout GrainCosmosAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Basic parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "delay_time",
        "Delay Time",
        juce::NormalisableRange<float>(0.1f, 2.0f, 0.01f),
        0.5f,
        "ms"));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "mix",
        "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        50.0f,
        "%"));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "feedback",
        "Feedback",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        50.0f,
        "%"));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "feedback_saturation",
        "Feedback Saturation",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        0.0f,
        "%"));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "grain_density",
        "Grain Density",
        juce::NormalisableRange<float>(1.0f, 32.0f, 1.0f),
        8.0f,
        "voices"));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "pitch_range",
        "Pitch Range",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        25.0f,
        "%"));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "buffer_position",
        "Buffer Position",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        0.0f,
        "%"));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "character",
        "Character",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        0.0f,
        "%"));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "chaos",
        "Chaos",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        0.0f,
        "%"));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "grain_size",
        "Grain Size",
        juce::NormalisableRange<float>(10.0f, 200.0f, 1.0f),
        50.0f,
        "ms"));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "envelope_shape",
        "Envelope Shape",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        ""));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "distortion_amount",
        "Distortion Amount",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        0.0f,
        "%"));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "freeze",
        "Freeze",
        juce::NormalisableRange<float>(0.0f, 1.0f, 1.0f),
        0.0f,
        ""));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "tempo_sync",
        "Tempo Sync",
        juce::NormalisableRange<float>(0.0f, 1.0f, 1.0f),
        0.0f,
        ""));

    // Additional parameters that are used but not fully implemented
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "reverse_probability",
        "Reverse Probability",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        0.0f,
        "%"));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "stereo_spread",
        "Stereo Spread",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        50.0f,
        "%"));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "attack_time",
        "Attack Time",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        10.0f,
        "%"));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "decay_time",
        "Decay Time",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        10.0f,
        "%"));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "shimmer",
        "Shimmer",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        0.0f,
        "%"));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "lofi",
        "Lo-Fi",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        0.0f,
        "%"));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GrainCosmosAudioProcessor();
}
