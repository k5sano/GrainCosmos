#include "PluginProcessor.h"

//==============================================================================
WebViewDemoAudioProcessor::WebViewDemoAudioProcessor()
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    parameters(*this, nullptr, "STATE", {
        // Cutoff frequency
        std::make_unique<juce::AudioParameterFloat>(
            "cutoff", "Cutoff",
            juce::NormalisableRange<float>{200.0f, 14000.0f, 1.0f, 0.5f},
            11000.0f, "Hz"
        ),
        // Mute
        std::make_unique<juce::AudioParameterBool>("mute", "Mute", false),
        // Filter type
        std::make_unique<juce::AudioParameterChoice>(
            "filterType", "Filter Type",
            juce::StringArray{"Low-pass", "High-pass", "Band-pass"},
            0
        )
    })
{
}

WebViewDemoAudioProcessor::~WebViewDemoAudioProcessor()
{
}

//==============================================================================
void WebViewDemoAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    const auto channels = std::max(getTotalNumInputChannels(), getTotalNumOutputChannels());
    if (channels == 0)
        return;

    filter.prepare({sampleRate, static_cast<uint32_t>(samplesPerBlock), static_cast<uint32_t>(channels)});
    filter.reset();
}

void WebViewDemoAudioProcessor::releaseResources()
{
}

void WebViewDemoAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalNumInputChannels = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    auto* cutoffParam = parameters.getParameter("cutoff");
    auto* muteParam = parameters.getParameter("mute");
    auto* filterTypeParam = parameters.getParameter("filterType");

    if (cutoffParam)
        filter.setCutoffFrequencyHz(cutoffParam->getValue());

    // Set filter mode
    const auto filterMode = [this, filterTypeParam]()
    {
        if (!filterTypeParam)
            return juce::dsp::LadderFilter<float>::Mode::LPF12;

        switch (static_cast<int>(filterTypeParam->getValue()))
        {
            case 0: return juce::dsp::LadderFilter<float>::Mode::LPF12;
            case 1: return juce::dsp::LadderFilter<float>::Mode::HPF12;
            default: return juce::dsp::LadderFilter<float>::Mode::BPF12;
        }
    }();
    filter.setMode(filterMode);

    auto outBlock = juce::dsp::AudioBlock<float>{buffer}.getSubsetChannelBlock(0, static_cast<size_t>(getTotalNumOutputChannels()));

    if (muteParam && muteParam->getValue() > 0.5f)
        outBlock.clear();

    filter.process(juce::dsp::ProcessContextReplacing<float>{outBlock});
}

//==============================================================================
juce::AudioProcessorEditor* WebViewDemoAudioProcessor::createEditor()
{
    return new WebViewDemoAudioProcessorEditor(*this);
}

//==============================================================================
void WebViewDemoAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, false);
    parameters.state.writeToStream(stream);
}

void WebViewDemoAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, static_cast<size_t>(sizeInBytes));
    if (tree.isValid())
        parameters.state = juce::AudioProcessorValueTreeState(tree, nullptr);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WebViewDemoAudioProcessor();
}
