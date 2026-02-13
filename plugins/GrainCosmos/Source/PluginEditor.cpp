#include "PluginEditor.h"

GrainCosmosAudioProcessorEditor::GrainCosmosAudioProcessorEditor(GrainCosmosAudioProcessor& p)
    : AudioProcessorEditor(p)
    , processorRef(p)
{
    // setSize is called LAST
    setSize(800, 500);
}

GrainCosmosAudioProcessorEditor::~GrainCosmosAudioProcessorEditor()
{
}

void GrainCosmosAudioProcessorEditor::resized()
{
    // Empty for now
}
