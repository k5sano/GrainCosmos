#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class DroneCosmosAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    DroneCosmosAudioProcessorEditor(DroneCosmosAudioProcessor&);
    ~DroneCosmosAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // Preset management
    juce::File getPresetsFolder();
    juce::File getOscPresetFolder(const juce::String& oscId);
    void scanPresets();
    void saveGlobalPreset();
    void saveOscPreset(const juce::String& oscId);
    void loadGlobalPreset(const juce::File& file);
    void loadOscPreset(const juce::File& file, const juce::String& oscId);

private:
    DroneCosmosAudioProcessor& audioProcessor;

    // UI Components
    juce::Label titleLabel;

    // Global preset
    juce::ComboBox globalPresetSelector;
    juce::TextButton globalSaveButton;
    juce::StringArray globalUserPresetNames;
    juce::Array<juce::File> globalUserPresetFiles;

    // Oscillator presets
    juce::ComboBox oscAPresetSelector;
    juce::TextButton oscASaveButton;
    juce::StringArray oscAUserPresetNames;
    juce::Array<juce::File> oscAUserPresetFiles;

    juce::ComboBox oscBPresetSelector;
    juce::TextButton oscBSaveButton;
    juce::StringArray oscBUserPresetNames;
    juce::Array<juce::File> oscBUserPresetFiles;

    juce::ComboBox oscCPresetSelector;
    juce::TextButton oscCSaveButton;
    juce::StringArray oscCUserPresetNames;
    juce::Array<juce::File> oscCUserPresetFiles;

    juce::ComboBox oscDPresetSelector;
    juce::TextButton oscDSaveButton;
    juce::StringArray oscDUserPresetNames;
    juce::Array<juce::File> oscDUserPresetFiles;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DroneCosmosAudioProcessorEditor)
};
