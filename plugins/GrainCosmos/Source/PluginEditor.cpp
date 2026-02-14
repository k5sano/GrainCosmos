#include "PluginEditor.h"

GrainCosmosAudioProcessorEditor::GrainCosmosAudioProcessorEditor(GrainCosmosAudioProcessor& p)
    : AudioProcessorEditor(p)
    , processorRef(p)
{
    // Title
    titleLabel.setText("GrainCosmos", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    // Preset selector
    presetSelector.addItem("Default", 1);
    presetSelector.addItem("Subtle Texture", 2);
    presetSelector.addItem("Glitch Machine", 3);
    presetSelector.addItem("Fuzz Madness", 4);
    presetSelector.addItem("Frozen Ambient", 5);
    presetSelector.addItem("Rhythmic Delay", 6);
    presetSelector.setSelectedId(1, juce::dontSendNotification);
    presetSelector.onChange = [this]() { loadPreset(presetSelector.getSelectedId()); };
    addAndMakeVisible(presetSelector);

    // Mix slider
    mixSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 18);
    addAndMakeVisible(mixSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, "mix", mixSlider);
    mixLabel.setText("Mix", juce::dontSendNotification);
    mixLabel.setJustificationType(juce::Justification::centredLeft);
    mixLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(mixLabel);

    // Distortion slider
    distortionSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    distortionSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 18);
    addAndMakeVisible(distortionSlider);
    distortionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, "distortion_amount", distortionSlider);
    distortionLabel.setText("Dist", juce::dontSendNotification);
    distortionLabel.setJustificationType(juce::Justification::centredLeft);
    distortionLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(distortionLabel);

    // Volume slider
    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 18);
    addAndMakeVisible(volumeSlider);
    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, "output_volume", volumeSlider);
    volumeLabel.setText("Vol", juce::dontSendNotification);
    volumeLabel.setJustificationType(juce::Justification::centredLeft);
    volumeLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(volumeLabel);

    // Threshold slider
    thresholdSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    thresholdSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 18);
    addAndMakeVisible(thresholdSlider);
    thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, "limiter_threshold", thresholdSlider);
    thresholdLabel.setText("Thresh", juce::dontSendNotification);
    thresholdLabel.setJustificationType(juce::Justification::centredLeft);
    thresholdLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(thresholdLabel);

    // Release slider
    releaseSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    releaseSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 18);
    addAndMakeVisible(releaseSlider);
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.parameters, "limiter_release", releaseSlider);
    releaseLabel.setText("Release", juce::dontSendNotification);
    releaseLabel.setJustificationType(juce::Justification::centredLeft);
    releaseLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(releaseLabel);

    // XY Pads
    xyPad1 = std::make_unique<XYPadPanel>(processorRef.parameters);
    xyPad2 = std::make_unique<XYPadPanel>(processorRef.parameters);
    xyPad3 = std::make_unique<XYPadPanel>(processorRef.parameters);
    xyPad4 = std::make_unique<XYPadPanel>(processorRef.parameters);
    addAndMakeVisible(xyPad1.get());
    addAndMakeVisible(xyPad2.get());
    addAndMakeVisible(xyPad3.get());
    addAndMakeVisible(xyPad4.get());

    // Pad 1: Chaos x Character
    xyPad1->setDefaultAxes(1, 2);
    // Pad 2: Delay Time x Grain Size
    xyPad2->setDefaultAxes(4, 3);
    // Pad 3: Envelope x Grain Voices
    xyPad3->setDefaultAxes(8, 9);
    // Pad 4: Feedback x Distortion
    xyPad4->setDefaultAxes(5, 7);

    setSize(1350, 700);
}

GrainCosmosAudioProcessorEditor::~GrainCosmosAudioProcessorEditor()
{
}

void GrainCosmosAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(30, 30, 35));
}

void GrainCosmosAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    int headerHeight = 60;
    int padding = 10;

    // Header area
    auto header = bounds.removeFromTop(headerHeight);

    // Title on the left
    titleLabel.setBounds(header.removeFromLeft(150).reduced(10, 5));

    // Preset selector between title and sliders
    auto presetArea = header.removeFromLeft(160);
    presetSelector.setBounds(presetArea.reduced(5, 15));

    header.removeFromLeft(10);

    // Each slider row: label (35px) + slider (remaining)
    int sliderGroupWidth = 200;
    int labelWidth = 35;
    int sliderHeight = 20;
    int sliderY = (headerHeight - sliderHeight) / 2;

    // Mix
    auto mixArea = header.removeFromLeft(sliderGroupWidth);
    mixLabel.setBounds(mixArea.getX(), sliderY, labelWidth, sliderHeight);
    mixSlider.setBounds(mixArea.getX() + labelWidth, sliderY, sliderGroupWidth - labelWidth, sliderHeight);

    header.removeFromLeft(10);

    // Distortion
    auto distArea = header.removeFromLeft(sliderGroupWidth);
    distortionLabel.setBounds(distArea.getX(), sliderY, labelWidth, sliderHeight);
    distortionSlider.setBounds(distArea.getX() + labelWidth, sliderY, sliderGroupWidth - labelWidth, sliderHeight);

    header.removeFromLeft(10);

    // Volume
    auto volArea = header.removeFromLeft(sliderGroupWidth);
    volumeLabel.setBounds(volArea.getX(), sliderY, labelWidth, sliderHeight);
    volumeSlider.setBounds(volArea.getX() + labelWidth, sliderY, sliderGroupWidth - labelWidth, sliderHeight);

    header.removeFromLeft(10);

    // Threshold
    auto threshArea = header.removeFromLeft(sliderGroupWidth);
    thresholdLabel.setBounds(threshArea.getX(), sliderY, labelWidth, sliderHeight);
    thresholdSlider.setBounds(threshArea.getX() + labelWidth, sliderY, sliderGroupWidth - labelWidth, sliderHeight);

    header.removeFromLeft(10);

    // Release
    auto releaseArea = header.removeFromLeft(sliderGroupWidth);
    releaseLabel.setBounds(releaseArea.getX(), sliderY, labelWidth, sliderHeight);
    releaseSlider.setBounds(releaseArea.getX() + labelWidth, sliderY, sliderGroupWidth - labelWidth, sliderHeight);

    // XY Pads - 2x2 grid
    auto content = bounds.reduced(padding);
    int panelWidth = (content.getWidth() - padding) / 2;
    int panelHeight = (content.getHeight() - padding) / 2;

    xyPad1->setBounds(content.getX(), content.getY(), panelWidth, panelHeight);
    xyPad2->setBounds(content.getX() + panelWidth + padding, content.getY(), panelWidth, panelHeight);
    xyPad3->setBounds(content.getX(), content.getY() + panelHeight + padding, panelWidth, panelHeight);
    xyPad4->setBounds(content.getX() + panelWidth + padding, content.getY() + panelHeight + padding, panelWidth, panelHeight);
}

void GrainCosmosAudioProcessorEditor::loadPreset(int presetId)
{
    auto& params = processorRef.parameters;

    auto setParam = [&](const juce::String& id, float value)
    {
        if (auto* p = params.getParameter(id))
        {
            auto range = p->getNormalisableRange();
            p->setValueNotifyingHost(range.convertTo0to1(value));
        }
    };

    switch (presetId)
    {
        case 1: // Default
            setParam("delay_time", 0.5f);
            setParam("grain_size", 100.0f);
            setParam("envelope_shape", 0.5f);
            setParam("distortion_amount", 0.0f);
            setParam("feedback", 40.0f);
            setParam("feedback_saturation", 0.0f);
            setParam("chaos", 20.0f);
            setParam("character", 50.0f);
            setParam("mix", 30.0f);
            setParam("freeze", 0.0f);
            setParam("tempo_sync", 0.0f);
            setParam("grain_voices", 32.0f);
            setParam("output_volume", 100.0f);
            setParam("limiter_threshold", -1.0f);
            setParam("limiter_release", 100.0f);
            break;

        case 2: // Subtle Texture
            setParam("delay_time", 0.8f);
            setParam("grain_size", 200.0f);
            setParam("envelope_shape", 0.9f);
            setParam("distortion_amount", 0.0f);
            setParam("feedback", 25.0f);
            setParam("feedback_saturation", 0.0f);
            setParam("chaos", 10.0f);
            setParam("character", 30.0f);
            setParam("mix", 20.0f);
            setParam("freeze", 0.0f);
            setParam("tempo_sync", 0.0f);
            setParam("grain_voices", 24.0f);
            setParam("output_volume", 100.0f);
            setParam("limiter_threshold", -1.0f);
            setParam("limiter_release", 100.0f);
            break;

        case 3: // Glitch Machine
            setParam("delay_time", 0.15f);
            setParam("grain_size", 15.0f);
            setParam("envelope_shape", 0.05f);
            setParam("distortion_amount", 30.0f);
            setParam("feedback", 60.0f);
            setParam("feedback_saturation", 20.0f);
            setParam("chaos", 85.0f);
            setParam("character", 80.0f);
            setParam("mix", 50.0f);
            setParam("freeze", 0.0f);
            setParam("tempo_sync", 0.0f);
            setParam("grain_voices", 48.0f);
            setParam("output_volume", 85.0f);
            setParam("limiter_threshold", -3.0f);
            setParam("limiter_release", 50.0f);
            break;

        case 4: // Fuzz Madness
            setParam("delay_time", 0.4f);
            setParam("grain_size", 80.0f);
            setParam("envelope_shape", 0.3f);
            setParam("distortion_amount", 70.0f);
            setParam("feedback", 85.0f);
            setParam("feedback_saturation", 75.0f);
            setParam("chaos", 60.0f);
            setParam("character", 70.0f);
            setParam("mix", 65.0f);
            setParam("freeze", 0.0f);
            setParam("tempo_sync", 0.0f);
            setParam("grain_voices", 40.0f);
            setParam("output_volume", 70.0f);
            setParam("limiter_threshold", -6.0f);
            setParam("limiter_release", 80.0f);
            break;

        case 5: // Frozen Ambient
            setParam("delay_time", 1.2f);
            setParam("grain_size", 300.0f);
            setParam("envelope_shape", 1.0f);
            setParam("distortion_amount", 0.0f);
            setParam("feedback", 70.0f);
            setParam("feedback_saturation", 0.0f);
            setParam("chaos", 15.0f);
            setParam("character", 20.0f);
            setParam("mix", 60.0f);
            setParam("freeze", 1.0f);
            setParam("tempo_sync", 0.0f);
            setParam("grain_voices", 32.0f);
            setParam("output_volume", 90.0f);
            setParam("limiter_threshold", -2.0f);
            setParam("limiter_release", 200.0f);
            break;

        case 6: // Rhythmic Delay
            setParam("delay_time", 0.5f);
            setParam("grain_size", 60.0f);
            setParam("envelope_shape", 0.15f);
            setParam("distortion_amount", 10.0f);
            setParam("feedback", 55.0f);
            setParam("feedback_saturation", 10.0f);
            setParam("chaos", 5.0f);
            setParam("character", 40.0f);
            setParam("mix", 40.0f);
            setParam("freeze", 0.0f);
            setParam("tempo_sync", 1.0f);
            setParam("grain_voices", 16.0f);
            setParam("output_volume", 100.0f);
            setParam("limiter_threshold", -1.0f);
            setParam("limiter_release", 100.0f);
            break;
    }
}

