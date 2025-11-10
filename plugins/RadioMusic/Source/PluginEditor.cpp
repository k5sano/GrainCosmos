#include "PluginEditor.h"

RadioMusicAudioProcessorEditor::RadioMusicAudioProcessorEditor(RadioMusicAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setSize(700, 500);
}

RadioMusicAudioProcessorEditor::~RadioMusicAudioProcessorEditor()
{
}

void RadioMusicAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawFittedText("RadioMusic - Stage 2", getLocalBounds(), juce::Justification::centred, 1);
}

void RadioMusicAudioProcessorEditor::resized()
{
}
