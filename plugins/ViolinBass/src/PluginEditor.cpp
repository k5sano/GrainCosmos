#include "PluginEditor.h"

//==============================================================================
ViolinBassAudioProcessorEditor::ViolinBassAudioProcessorEditor(ViolinBassAudioProcessor& p)
    : AudioProcessorEditor(p), audioProcessor(p)
{
    // Set UI size
    setSize(400, 500);

    // Title
    titleLabel.setText("ViolinBass", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(28.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    // Pitch slider (semitones)
    pitchSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    pitchSlider.setRange(-24.0, 24.0, 0.1);
    pitchSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    pitchSlider.setValue(-12.0, juce::dontSendNotification);
    addAndMakeVisible(pitchSlider);
    pitchAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "pitch_semitones", pitchSlider);

    pitchLabel.setText("Pitch (st)", juce::dontSendNotification);
    pitchLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(pitchLabel);

    // Fine tune slider (cents)
    fineTuneSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    fineTuneSlider.setRange(-100.0, 100.0);
    fineTuneSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    addAndMakeVisible(fineTuneSlider);
    fineTuneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "fine_tune", fineTuneSlider);

    fineTuneLabel.setText("Fine (ct)", juce::dontSendNotification);
    fineTuneLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(fineTuneLabel);

    // Formant preserve toggle
    formantToggle.setButtonText("Formant Preserve");
    formantToggle.setToggleState(true, juce::dontSendNotification);
    addAndMakeVisible(formantToggle);
    formantAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.parameters, "formant_preserve", formantToggle);

    formantLabel.setText("Formant Preserve", juce::dontSendNotification);
    formantLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(formantLabel);

    // Dry/Wet slider
    dryWetSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    dryWetSlider.setRange(0.0, 100.0);
    dryWetSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    dryWetSlider.setValue(100.0, juce::dontSendNotification);
    addAndMakeVisible(dryWetSlider);
    dryWetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "dry_wet", dryWetSlider);

    dryWetLabel.setText("Dry/Wet %", juce::dontSendNotification);
    dryWetLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(dryWetLabel);

    // Output gain slider
    outputGainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    outputGainSlider.setRange(-12.0, 12.0);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    outputGainSlider.setValue(0.0, juce::dontSendNotification);
    addAndMakeVisible(outputGainSlider);
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "output_gain", outputGainSlider);

    outputGainLabel.setText("Output (dB)", juce::dontSendNotification);
    outputGainLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(outputGainLabel);

    // LPF cutoff slider
    lpfCutoffSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    lpfCutoffSlider.setRange(2000.0, 8000.0);
    lpfCutoffSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    lpfCutoffSlider.setValue(8000.0, juce::dontSendNotification);
    addAndMakeVisible(lpfCutoffSlider);
    lpfCutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "lpf_cutoff", lpfCutoffSlider);

    lpfCutoffLabel.setText("LPF (Hz)", juce::dontSendNotification);
    lpfCutoffLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(lpfCutoffLabel);

    // HPF cutoff slider
    hpfCutoffSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    hpfCutoffSlider.setRange(20.0, 80.0);
    hpfCutoffSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    hpfCutoffSlider.setValue(40.0, juce::dontSendNotification);
    addAndMakeVisible(hpfCutoffSlider);
    hpfCutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "hpf_cutoff", hpfCutoffSlider);

    hpfCutoffLabel.setText("HPF (Hz)", juce::dontSendNotification);
    hpfCutoffLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(hpfCutoffLabel);

    // Level meters
    inputMeterComponent.setSize(10, 100);
    addAndMakeVisible(inputMeterComponent);

    outputMeterComponent.setSize(10, 100);
    addAndMakeVisible(outputMeterComponent);

    // Latency display
    latencyLabel.setText("Latency: -- ms", juce::dontSendNotification);
    latencyLabel.setColour(juce::Label::textColourId, juce::Colours::green);
    latencyLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(latencyLabel);

    // Start UI update timer (30Hz refresh)
    startTimerHz(30);
}

ViolinBassAudioProcessorEditor::~ViolinBassAudioProcessorEditor()
{
}

//==============================================================================
void ViolinBassAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    // Draw background gradient
    juce::ColourGradient gradient(juce::Colours::darkgrey.withBrightness(0.1f), 0, 0,
                                   juce::Colours::black.withBrightness(0.0f), 0, (float)getHeight(), false);
    g.setGradientFill(gradient);
    g.fillAll();

    // Draw level meter backgrounds
    auto meterArea = getLocalBounds().toFloat().reduced(20);
    meterArea = meterArea.removeFromRight(30);
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(meterArea.withTrimmedTop(50).withHeight(100));
    g.fillRect(meterArea.withTrimmedTop(170).withHeight(100));
}

void ViolinBassAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);

    // Title
    titleLabel.setBounds(area.removeFromTop(50));

    // Main controls area
    auto controlsArea = area.removeFromTop(350);

    int sliderHeight = 40;
    int labelHeight = 20;
    int spacing = 15;

    // Pitch controls
    auto pitchArea = controlsArea.removeFromTop(sliderHeight + labelHeight + spacing);
    pitchSlider.setBounds(pitchArea.removeFromTop(sliderHeight));
    pitchLabel.setBounds(pitchArea.removeFromTop(labelHeight));

    controlsArea.removeFromTop(spacing);

    auto fineTuneArea = controlsArea.removeFromTop(sliderHeight + labelHeight + spacing);
    fineTuneSlider.setBounds(fineTuneArea.removeFromTop(sliderHeight));
    fineTuneLabel.setBounds(fineTuneArea.removeFromTop(labelHeight));

    controlsArea.removeFromTop(spacing);

    // Formant toggle
    auto formantArea = controlsArea.removeFromTop(30 + labelHeight + spacing);
    formantToggle.setBounds(formantArea.removeFromTop(30));
    formantLabel.setBounds(formantArea.removeFromTop(labelHeight));

    controlsArea.removeFromTop(spacing);

    // Filters
    auto lpfArea = controlsArea.removeFromTop(sliderHeight + labelHeight + spacing);
    lpfCutoffSlider.setBounds(lpfArea.removeFromTop(sliderHeight));
    lpfCutoffLabel.setBounds(lpfArea.removeFromTop(labelHeight));

    controlsArea.removeFromTop(spacing);

    auto hpfArea = controlsArea.removeFromTop(sliderHeight + labelHeight + spacing);
    hpfCutoffSlider.setBounds(hpfArea.removeFromTop(sliderHeight));
    hpfCutoffLabel.setBounds(hpfArea.removeFromTop(labelHeight));

    controlsArea.removeFromTop(spacing);

    // Dry/Wet
    auto dryWetArea = controlsArea.removeFromTop(sliderHeight + labelHeight + spacing);
    dryWetSlider.setBounds(dryWetArea.removeFromTop(sliderHeight));
    dryWetLabel.setBounds(dryWetArea.removeFromTop(labelHeight));

    controlsArea.removeFromTop(spacing);

    // Output gain
    auto gainArea = controlsArea.removeFromTop(sliderHeight + labelHeight + spacing);
    outputGainSlider.setBounds(gainArea.removeFromTop(sliderHeight));
    outputGainLabel.setBounds(gainArea.removeFromTop(labelHeight));

    // Latency display
    latencyLabel.setBounds(area.removeFromTop(30));

    // Level meters on the right
    auto meterArea = getLocalBounds().reduced(20);
    inputMeterComponent.setBounds(meterArea.withTrimmedTop(50).withHeight(100));
    outputMeterComponent.setBounds(meterArea.withTrimmedTop(170).withHeight(100));
}

void ViolinBassAudioProcessorEditor::updateLevelMeters()
{
    // Get current levels from processor
    float inputLevel = audioProcessor.currentInputLevel;
    float outputLevel = audioProcessor.currentOutputLevel;

    // Convert to 0-1 range for display (dB scale approximation)
    auto getMeterLevel = [](float level) -> float
    {
        float dB = 20.0f * std::log10(level);
        return juce::jmap(juce::jlimit(-60.0f, 0.0f, dB), -60.0f, 0.0f, 0.0f, 1.0f);
    };

    inputMeterLevel = getMeterLevel(inputLevel);
    outputMeterLevel = getMeterLevel(outputLevel);

    // Update meter components
    inputMeterComponent.setLevel(inputMeterLevel);
    outputMeterComponent.setLevel(outputMeterLevel);
}

void ViolinBassAudioProcessorEditor::updateLatencyDisplay()
{
    // Get current latency from pitch shifter
    double latencyMs = audioProcessor.getPitchShifter().getLatencyMs();
    latencyLabel.setText(juce::String::formatted("Latency: %.1f ms", latencyMs),
                       juce::dontSendNotification);
}

void ViolinBassAudioProcessorEditor::timerCallback()
{
    updateLevelMeters();
    updateLatencyDisplay();
}
