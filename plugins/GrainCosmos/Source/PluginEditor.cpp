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

    setSize(1200, 700);
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

