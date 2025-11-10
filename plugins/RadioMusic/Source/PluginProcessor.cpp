#include "PluginProcessor.h"
#include "PluginEditor.h"

RadioMusicAudioProcessor::RadioMusicAudioProcessor()
    : AudioProcessor(BusesProperties()
                        .withInput("Input", juce::AudioChannelSet::stereo(), true)
                        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

RadioMusicAudioProcessor::~RadioMusicAudioProcessor()
{
}

void RadioMusicAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void RadioMusicAudioProcessor::releaseResources()
{
}

void RadioMusicAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    // Pass-through for Stage 2 (DSP added in Stage 4)
}

juce::AudioProcessorEditor* RadioMusicAudioProcessor::createEditor()
{
    return new RadioMusicAudioProcessorEditor(*this);
}

void RadioMusicAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::ignoreUnused(destData);
}

void RadioMusicAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::ignoreUnused(data, sizeInBytes);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RadioMusicAudioProcessor();
}
