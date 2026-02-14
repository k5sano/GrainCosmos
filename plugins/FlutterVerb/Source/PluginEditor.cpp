#include "PluginEditor.h"

//==============================================================================
FlutterVerbAudioProcessorEditor::FlutterVerbAudioProcessorEditor(FlutterVerbAudioProcessor& p)
    : AudioProcessorEditor(p), audioProcessor(p)
{
    setSize(400, 300);
}

FlutterVerbAudioProcessorEditor::~FlutterVerbAudioProcessorEditor()
{
}

//==============================================================================
void FlutterVerbAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawFittedText("FlutterVerb", getLocalBounds(), juce::Justification::centred, 1);
}

void FlutterVerbAudioProcessorEditor::resized()
{
}
