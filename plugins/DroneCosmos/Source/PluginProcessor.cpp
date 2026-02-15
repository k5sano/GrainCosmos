#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout DroneCosmosAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Oscillator A (4 params)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "osc_a_waveform", "OSC A Waveform",
        juce::NormalisableRange<float>(0.0f, 3.0f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("")));
    layout.add(std::make_unique<juce::AudioParameterInt>(
        "osc_a_pitch", "OSC A Pitch",
        -24, 24, 0));
    layout.add(std::make_unique<juce::AudioParameterInt>(
        "osc_a_detune", "OSC A Detune",
        -50, 50, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "osc_a_level", "OSC A Level",
        juce::NormalisableRange<float>(0.0f, 100.0f), 75.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Oscillator B (4 params)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "osc_b_waveform", "OSC B Waveform",
        juce::NormalisableRange<float>(0.0f, 3.0f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterInt>(
        "osc_b_pitch", "OSC B Pitch",
        -24, 24, 0));
    layout.add(std::make_unique<juce::AudioParameterInt>(
        "osc_b_detune", "OSC B Detune",
        -50, 50, 7));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "osc_b_level", "OSC B Level",
        juce::NormalisableRange<float>(0.0f, 100.0f), 75.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Oscillator C (4 params)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "osc_c_waveform", "OSC C Waveform",
        juce::NormalisableRange<float>(0.0f, 3.0f), 1.0f));
    layout.add(std::make_unique<juce::AudioParameterInt>(
        "osc_c_pitch", "OSC C Pitch",
        -24, 24, -12));
    layout.add(std::make_unique<juce::AudioParameterInt>(
        "osc_c_detune", "OSC C Detune",
        -50, 50, -5));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "osc_c_level", "OSC C Level",
        juce::NormalisableRange<float>(0.0f, 100.0f), 50.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Oscillator D (4 params)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "osc_d_waveform", "OSC D Waveform",
        juce::NormalisableRange<float>(0.0f, 3.0f), 2.0f));
    layout.add(std::make_unique<juce::AudioParameterInt>(
        "osc_d_pitch", "OSC D Pitch",
        -24, 24, 7));
    layout.add(std::make_unique<juce::AudioParameterInt>(
        "osc_d_detune", "OSC D Detune",
        -50, 50, 3));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "osc_d_level", "OSC D Level",
        juce::NormalisableRange<float>(0.0f, 100.0f), 50.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Modulation Matrix (6 params)
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"mod_range", 1}, "Mod Range",
        juce::StringArray{"Low", "Mid", "High"}, 1));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "mod_mode", "Modulation Mode",
        juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "self_mod", "Self Modulation",
        juce::NormalisableRange<float>(0.0f, 100.0f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "cross_mod", "Cross Modulation",
        juce::NormalisableRange<float>(0.0f, 100.0f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "ring_mod", "Ring Modulation",
        juce::NormalisableRange<float>(0.0f, 100.0f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "chaos_mod", "Chaos Modulation",
        juce::NormalisableRange<float>(0.0f, 100.0f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Drone Pitch (1 param)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "drone_pitch", "Drone Base Pitch",
        juce::NormalisableRange<float>(20.0f, 500.0f), 55.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    // Output (3 params)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "output_volume", "Output Volume",
        juce::NormalisableRange<float>(0.0f, 100.0f), 80.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "limiter_threshold", "Limiter Threshold",
        juce::NormalisableRange<float>(-20.0f, 0.0f), -1.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "limiter_release", "Limiter Release",
        juce::NormalisableRange<float>(10.0f, 500.0f), 100.0f,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    return layout;
}

//==============================================================================
DroneCosmosAudioProcessor::DroneCosmosAudioProcessor()
    : juce::AudioProcessor(BusesProperties()
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , parameters(*this, nullptr, juce::Identifier("DroneCosmos"), createParameterLayout())
{
}

DroneCosmosAudioProcessor::~DroneCosmosAudioProcessor()
{
}

//==============================================================================
void DroneCosmosAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    // Initialize oscillator states
    for (auto& osc : oscStates)
    {
        osc.phase = 0.0f;
        osc.phaseIncrement = 0.0f;
        osc.dcFilterZ1 = 0.0f;
    }

    // Chaos LFO: slow random modulation (~0.5 Hz)
    chaosLFOIncrement = 0.5f / static_cast<float>(sampleRate);

    // Prepare limiter
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 2;
    limiter.prepare(spec);
    limiter.setThreshold(-1.0f);
    limiter.setRelease(100.0f);
}

void DroneCosmosAudioProcessor::releaseResources()
{
}

float DroneCosmosAudioProcessor::generateWaveform(float phase, float waveformParam)
{
    // waveformParam: 0.0 = sine, 1.0 = saw, 2.0 = square, 3.0 = triangle
    // Interpolate between adjacent waveforms

    float phasePi = phase * juce::MathConstants<float>::twoPi;

    float sine = std::sin(phasePi);
    float saw = 2.0f * phase - 1.0f;
    float square = (phase < 0.5f) ? 1.0f : -1.0f;
    float triangle = 2.0f * std::abs(2.0f * phase - 1.0f) - 1.0f;

    if (waveformParam < 1.0f)
    {
        // Sine -> Saw
        float t = waveformParam;
        return sine * (1.0f - t) + saw * t;
    }
    else if (waveformParam < 2.0f)
    {
        // Saw -> Square
        float t = waveformParam - 1.0f;
        return saw * (1.0f - t) + square * t;
    }
    else
    {
        // Square -> Triangle
        float t = waveformParam - 2.0f;
        return square * (1.0f - t) + triangle * t;
    }
}

float DroneCosmosAudioProcessor::softClip(float sample)
{
    // Soft clipping to prevent harsh aliasing
    return std::tanh(sample);
}

float DroneCosmosAudioProcessor::dcFilter(float sample, float& z1, float rc)
{
    // 1st order highpass filter: y[n] = rc * (y[n-1] + x[n] - x[n-1])
    // rc = 1 - (cutoff * 2pi / sampleRate)
    // For ~20Hz at 44100Hz: rc ≈ 0.997
    float y = rc * (z1 + sample - z1);
    z1 = sample;
    return y;
}

void DroneCosmosAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Get parameter values (atomic)
    auto* oscAWaveform = parameters.getRawParameterValue("osc_a_waveform");
    auto* oscAPitch = parameters.getRawParameterValue("osc_a_pitch");
    auto* oscADetune = parameters.getRawParameterValue("osc_a_detune");
    auto* oscALevel = parameters.getRawParameterValue("osc_a_level");

    auto* oscBWaveform = parameters.getRawParameterValue("osc_b_waveform");
    auto* oscBPitch = parameters.getRawParameterValue("osc_b_pitch");
    auto* oscBDetune = parameters.getRawParameterValue("osc_b_detune");
    auto* oscBLevel = parameters.getRawParameterValue("osc_b_level");

    auto* oscCWaveform = parameters.getRawParameterValue("osc_c_waveform");
    auto* oscCPitch = parameters.getRawParameterValue("osc_c_pitch");
    auto* oscCDetune = parameters.getRawParameterValue("osc_c_detune");
    auto* oscCLevel = parameters.getRawParameterValue("osc_c_level");

    auto* oscDWaveform = parameters.getRawParameterValue("osc_d_waveform");
    auto* oscDPitch = parameters.getRawParameterValue("osc_d_pitch");
    auto* oscDDetune = parameters.getRawParameterValue("osc_d_detune");
    auto* oscDLevel = parameters.getRawParameterValue("osc_d_level");

    auto* modRange = parameters.getRawParameterValue("mod_range");
    auto* modMode = parameters.getRawParameterValue("mod_mode");
    auto* selfMod = parameters.getRawParameterValue("self_mod");
    auto* crossMod = parameters.getRawParameterValue("cross_mod");
    auto* ringMod = parameters.getRawParameterValue("ring_mod");
    auto* chaosMod = parameters.getRawParameterValue("chaos_mod");

    auto* dronePitch = parameters.getRawParameterValue("drone_pitch");
    auto* outputVolume = parameters.getRawParameterValue("output_volume");
    auto* limiterThreshold = parameters.getRawParameterValue("limiter_threshold");
    auto* limiterRelease = parameters.getRawParameterValue("limiter_release");

    // Update limiter
    limiter.setThreshold(*limiterThreshold);
    limiter.setRelease(*limiterRelease);

    const float baseFreq = *dronePitch;

    // Calculate frequency for each oscillator
    auto getFrequency = [&](int pitchSemitones, int detuneCents) -> float
    {
        float semitoneRatio = std::pow(2.0f, pitchSemitones / 12.0f);
        float centRatio = std::pow(2.0f, detuneCents / 1200.0f);
        return baseFreq * semitoneRatio * centRatio;
    };

    float freqA = getFrequency(static_cast<int>(*oscAPitch), static_cast<int>(*oscADetune));
    float freqB = getFrequency(static_cast<int>(*oscBPitch), static_cast<int>(*oscBDetune));
    float freqC = getFrequency(static_cast<int>(*oscCPitch), static_cast<int>(*oscCDetune));
    float freqD = getFrequency(static_cast<int>(*oscDPitch), static_cast<int>(*oscDDetune));

    std::array<float, 4> freqs = { freqA, freqB, freqC, freqD };

    // Modulation range scaling (L/M/H)
    // Low: subtle drone, Mid: balanced, High: noise machine
    int rangeIndex = static_cast<int>(std::round(*modRange));
    float rangeSelfScale, rangeCrossScale, rangeRingScale, rangeChaosScale;

    switch (rangeIndex)
    {
        case 0: // Low - delicate drone
            rangeSelfScale = 0.001f;   // 0-100 -> 0-0.1
            rangeCrossScale = 0.002f;  // 0-100 -> 0-0.2
            rangeRingScale = 0.002f;   // 0-100 -> 0-0.2
            rangeChaosScale = 0.0005f; // 0-100 -> 0-0.05
            break;
        case 1: // Mid - balanced (default)
            rangeSelfScale = 0.01f;    // 0-100 -> 0-1.0
            rangeCrossScale = 0.02f;   // 0-100 -> 0-2.0
            rangeRingScale = 0.02f;    // 0-100 -> 0-2.0
            rangeChaosScale = 0.003f;  // 0-100 -> 0-0.3
            break;
        case 2: // High - noise machine
            rangeSelfScale = 0.05f;    // 0-100 -> 0-5.0
            rangeCrossScale = 0.1f;    // 0-100 -> 0-10.0
            rangeRingScale = 0.1f;     // 0-100 -> 0-10.0
            rangeChaosScale = 0.02f;   // 0-100 -> 0-2.0
            break;
        default:
            rangeSelfScale = 0.01f;
            rangeCrossScale = 0.02f;
            rangeRingScale = 0.02f;
            rangeChaosScale = 0.003f;
            break;
    }

    // Modulation mode scaling
    // 0.0 = Drone Mode (subtle modulation)
    // 1.0 = Noise Mode (extreme modulation)
    float modModeValue = *modMode;
    float droneScale = 1.0f - modModeValue;
    float noiseScale = modModeValue;

    // Base modulation amounts (normalized 0-1 from parameter)
    float selfParam = *selfMod / 100.0f;
    float crossParam = *crossMod / 100.0f;
    float ringParam = *ringMod / 100.0f;
    float chaosParam = *chaosMod / 100.0f;

    // Apply range-based scaling first, then mode-based scaling
    // Range scaling maps 0-100 parameter to the appropriate range
    // Mode scaling provides additional Drone/Noise interpolation
    float selfRangeAmt = selfParam * 100.0f;  // Convert back to 0-100
    float crossRangeAmt = crossParam * 100.0f;
    float ringRangeAmt = ringParam * 100.0f;
    float chaosRangeAmt = chaosParam * 100.0f;

    // Apply range scales
    float selfAmt = selfRangeAmt * rangeSelfScale * (0.3f * droneScale + 5.0f * noiseScale);
    float crossAmt = crossRangeAmt * rangeCrossScale * (0.5f * droneScale + 8.0f * noiseScale);
    float ringAmt = ringRangeAmt * rangeRingScale * (0.5f * droneScale + 8.0f * noiseScale);
    float chaosAmt = chaosRangeAmt * rangeChaosScale * (0.1f * droneScale + 2.0f * noiseScale);

    // DC filter coefficient for ~20Hz highpass
    float dcFilterRC = 1.0f - (20.0f * 2.0f * juce::MathConstants<float>::pi / static_cast<float>(currentSampleRate));

    // Oscillator output levels
    float levelA = *oscALevel / 100.0f;
    float levelB = *oscBLevel / 100.0f;
    float levelC = *oscCLevel / 100.0f;
    float levelD = *oscDLevel / 100.0f;

    // Clear output buffer
    buffer.clear();

    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getWritePointer(1);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        // Update chaos LFO
        float chaosValue = 0.0f;
        if (chaosAmt > 0.0f)
        {
            chaosLFOPhase += chaosLFOIncrement;
            if (chaosLFOPhase >= 1.0f)
                chaosLFOPhase -= 1.0f;
            // Triangle LFO for smooth modulation
            chaosValue = (2.0f * std::abs(2.0f * chaosLFOPhase - 1.0f) - 1.0f) * chaosAmt;
        }

        // Calculate phase increments with FM
        std::array<float, 4> increments;
        std::array<float, 4> oscOutputs;

        // First pass: calculate base increments
        for (int i = 0; i < 4; ++i)
        {
            increments[i] = freqs[i] / static_cast<float>(currentSampleRate);
        }

        // Generate oscillator outputs
        for (int i = 0; i < 4; ++i)
        {
            float& phase = oscStates[i].phase;
            float waveParam = 0.0f;
            float level = 0.0f;

            switch (i)
            {
                case 0: waveParam = *oscAWaveform; level = levelA; break;
                case 1: waveParam = *oscBWaveform; level = levelB; break;
                case 2: waveParam = *oscCWaveform; level = levelC; break;
                case 3: waveParam = *oscDWaveform; level = levelD; break;
            }

            oscOutputs[i] = generateWaveform(phase, waveParam) * level;

            // Self-modulation: oscillator output modulates its own phase
            if (selfAmt > 0.0f)
            {
                float mod = selfAmt * oscOutputs[i] * (1.0f + chaosValue);
                phase += mod * 0.1f; // Scale down self-modulation

                // Clamp phase to prevent runaway/NaN/Inf
                // Ensure phase increment stays within reasonable bounds
                if (phase < 0.0f)
                    phase = std::fmod(std::abs(phase), 1.0f);
                if (phase > 0.5f)  // Nyquist/2 for safety
                    phase = 0.5f;
            }
        }

        // Cross-modulation: A<->B, C<->D
        if (crossAmt > 0.0f)
        {
            float crossScale = crossAmt * 0.05f * (1.0f + chaosValue);
            oscStates[0].phase += oscOutputs[1] * crossScale; // A modulated by B
            oscStates[1].phase += oscOutputs[0] * crossScale; // B modulated by A
            oscStates[2].phase += oscOutputs[3] * crossScale; // C modulated by D
            oscStates[3].phase += oscOutputs[2] * crossScale; // D modulated by C
        }

        // Ring-modulation: A->B->C->D->A
        if (ringAmt > 0.0f)
        {
            float ringScale = ringAmt * 0.03f * (1.0f + chaosValue);
            oscStates[1].phase += oscOutputs[0] * ringScale; // A->B
            oscStates[2].phase += oscOutputs[1] * ringScale; // B->C
            oscStates[3].phase += oscOutputs[2] * ringScale; // C->D
            oscStates[0].phase += oscOutputs[3] * ringScale; // D->A
        }

        // Advance phases and apply DC filter to each oscillator
        float mix = 0.0f;
        for (int i = 0; i < 4; ++i)
        {
            oscStates[i].phase += increments[i];
            if (oscStates[i].phase >= 1.0f)
                oscStates[i].phase -= 1.0f;

            // Apply DC filter to remove DC offset from modulation
            oscOutputs[i] = dcFilter(oscOutputs[i], oscStates[i].dcFilterZ1, dcFilterRC);

            // NaN/Inf safety check
            if (!std::isfinite(oscOutputs[i]))
                oscOutputs[i] = 0.0f;

            mix += oscOutputs[i];
        }

        // Soft clip to prevent aliasing
        mix = softClip(mix * 0.25f); // Normalize by number of oscillators

        // Apply output volume
        float vol = *outputVolume / 100.0f;
        mix *= vol;

        // Final NaN/Inf safety check
        if (!std::isfinite(mix))
            mix = 0.0f;

        leftChannel[sample] = mix;
        rightChannel[sample] = mix;
    }

    // Apply limiter
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    limiter.process(context);
}

//==============================================================================
juce::AudioProcessorEditor* DroneCosmosAudioProcessor::createEditor()
{
    return new DroneCosmosAudioProcessorEditor(*this);
}

//==============================================================================
void DroneCosmosAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void DroneCosmosAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DroneCosmosAudioProcessor();
}
