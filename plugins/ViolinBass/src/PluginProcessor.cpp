#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout ViolinBassAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Pitch Shift (semitones)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "pitch_semitones", "Pitch (semitones)",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), -12.0f,
        juce::AudioParameterFloatAttributes().withLabel("st")));

    // Fine Tune (cents)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "fine_tune", "Fine Tune (cents)",
        juce::NormalisableRange<float>(-100.0f, 100.0f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("ct")));

    // Formant Preserve (toggle)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        "formant_preserve", "Formant Preserve",
        true));

    // Dry/Wet Mix
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "dry_wet", "Dry/Wet Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f), 100.0f,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Output Gain (dB)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "output_gain", "Output Gain",
        juce::NormalisableRange<float>(-12.0f, 12.0f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // Low Pass Filter (cutoff frequency)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "lpf_cutoff", "LPF Cutoff",
        juce::NormalisableRange<float>(2000.0f, 8000.0f, 0.5f), 8000.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    // High Pass Filter (cutoff frequency)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "hpf_cutoff", "HPF Cutoff",
        juce::NormalisableRange<float>(20.0f, 80.0f, 0.5f), 40.0f,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    return layout;
}

//==============================================================================
ViolinBassAudioProcessor::ViolinBassAudioProcessor()
    : juce::AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , parameters(*this, nullptr, juce::Identifier("ViolinBass"), createParameterLayout())
{
}

ViolinBassAudioProcessor::~ViolinBassAudioProcessor()
{
}

//==============================================================================
void ViolinBassAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    DBG("ViolinBassAudioProcessor::prepareToPlay: sampleRate=" << sampleRate << ", samplesPerBlock=" << samplesPerBlock);

    // Prepare pitch shifter
    pitchShifter.prepare(sampleRate, samplesPerBlock);

    int latency = pitchShifter.getLatency();
    DBG("PitchShifter latency: " << latency << " samples");

    // Set latency for DAW compensation
    setLatencySamples(latency);
    DBG("Called setLatencySamples(" << latency << ")");

    // Prepare filters
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 2;

    lowPassFilter.prepare(spec);
    highPassFilter.prepare(spec);

    updateFilters();

    // Reset level meters
    inputLevelDecay = 0.0f;
    outputLevelDecay = 0.0f;
    currentInputLevel = 0.0f;
    currentOutputLevel = 0.0f;
}

void ViolinBassAudioProcessor::releaseResources()
{
}

void ViolinBassAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Get parameter values (atomic)
    auto* pitchSemitones = parameters.getRawParameterValue("pitch_semitones");
    auto* fineTune = parameters.getRawParameterValue("fine_tune");
    auto* formantPreserve = parameters.getRawParameterValue("formant_preserve");
    auto* outputGainParam = parameters.getRawParameterValue("output_gain");

    // Update pitch shifter settings
    double totalSemitones = *pitchSemitones + (*fineTune / 100.0);
    bool formantPreserved = *formantPreserve > 0.5f;

    // Only update when values actually change
    if (std::abs(totalSemitones - previousPitchSemitones) > 0.001)
    {
        pitchShifter.setPitchSemiTones(totalSemitones);
        previousPitchSemitones = totalSemitones;
    }

    if (formantPreserved != previousFormantPreserved)
    {
        pitchShifter.setFormantPreserved(formantPreserved);
        previousFormantPreserved = formantPreserved;
    }

    // Update filters
    updateFilters();

    // Calculate output gain
    float outputGain = juce::Decibels::decibelsToGain(outputGainParam->load());

    // Store dry signal for level metering (before processing)
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    // Process through pitch shifter (wet signal only - no dry mix to avoid phase interference)
    pitchShifter.process(buffer);

    // Apply filters to wet signal
    juce::dsp::AudioBlock<float> wetBlock(buffer);
    juce::dsp::ProcessContextReplacing<float> wetContext(wetBlock);

    highPassFilter.process(wetContext);
    lowPassFilter.process(wetContext);

    // Apply output gain (wet signal only)
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* wetChannel = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            wetChannel[sample] *= outputGain;
        }
    }

    // Update level meters (peak decay)
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            // Input level (from dry buffer)
            float inputSample = std::abs(dryBuffer.getReadPointer(channel)[sample]);
            currentInputLevel = calculateLevel(inputSample, inputLevelDecay);

            // Output level
            float outputSample = std::abs(buffer.getWritePointer(channel)[sample]);
            currentOutputLevel = calculateLevel(outputSample, outputLevelDecay);
        }
    }
}

void ViolinBassAudioProcessor::updateFilters()
{
    auto* lpfCutoff = parameters.getRawParameterValue("lpf_cutoff");
    auto* hpfCutoff = parameters.getRawParameterValue("hpf_cutoff");

    // Update low pass filter (remove high-frequency artifacts)
    lowPassFilter.setCutoffFrequency(lpfCutoff->load());
    lowPassFilter.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);

    // Update high pass filter (remove sub-bass)
    highPassFilter.setCutoffFrequency(hpfCutoff->load());
    highPassFilter.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
}

float ViolinBassAudioProcessor::calculateLevel(float sample, float& decay)
{
    // Peak decay meter with 30dB/sec decay rate
    const float decayRate = 0.993f; // Approx 30dB/sec at 48kHz

    if (sample > decay)
        decay = sample;
    else
        decay *= decayRate;

    return juce::jmax(decay, 0.00001f); // Return in dB scale
}

//==============================================================================
juce::AudioProcessorEditor* ViolinBassAudioProcessor::createEditor()
{
    return new ViolinBassAudioProcessorEditor(*this);
}

//==============================================================================
void ViolinBassAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ViolinBassAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ViolinBassAudioProcessor();
}
