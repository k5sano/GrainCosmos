#include "PluginProcessor.h"
#include "PluginEditor.h"

AbyssVerbAudioProcessorEditor::AbyssVerbAudioProcessorEditor(AbyssVerbAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(900, 620);

    auto warm = juce::Colour(0xFFB87A4B);   // 木の温もり
    auto deep = juce::Colour(0xFF3A7CA5);   // 深い青
    auto fade = juce::Colour(0xFF6B5B73);   // 消えゆく紫
    auto mix  = juce::Colour(0xFF4A9EBF);   // 標準

    // バイオリン入力セクション
    setupKnob(piezoKnob,      "piezoCorrect",  "PIEZO FIX",     warm);
    setupKnob(bodyKnob,       "bodyResonance", "BODY",          warm);
    setupKnob(brightnessKnob, "brightness",    "BRIGHTNESS",    warm);

    // リバーブ
    setupKnob(decayKnob,    "reverbDecay",    "ABYSS DEPTH",   deep);
    setupKnob(dampHighKnob, "reverbDampHigh", "HIGH DARKNESS",  deep);
    setupKnob(dampLowKnob,  "reverbDampLow",  "LOW WARMTH",    deep);
    setupKnob(shimmerKnob,  "reverbModDepth", "SHIMMER",       deep);
    setupKnob(swayKnob,     "reverbModRate",  "SWAY",          deep);

    // ディレイ
    setupKnob(echoTimeKnob,    "delayTime",      "ECHO TIME",     fade);
    setupKnob(echoSustainKnob, "delayFeedback",  "ECHO SUSTAIN",  fade);
    setupKnob(vanishKnob,      "vanishRate",      "VANISH",        fade);
    setupKnob(fadeTexKnob,     "degradeAmount",   "FADE TEXTURE",  fade);
    setupKnob(driftKnob,       "driftAmount",     "TIME DRIFT",    fade);
    setupKnob(chorusKnob,      "detuneAmount",    "CHORUS DRIFT",  fade);

    // ミックス
    setupKnob(reverbMixKnob, "reverbMix",      "ABYSS MIX",     mix);
    setupKnob(delayMixKnob,  "delayMix",       "ECHO MIX",      mix);
    setupKnob(masterMixKnob, "masterMix",      "DRY / WET",     mix);
    setupKnob(bowSensKnob,   "bowSensitivity", "BOW FEEL",      juce::Colour(0xFFCC8855));
}

AbyssVerbAudioProcessorEditor::~AbyssVerbAudioProcessorEditor() {}

void AbyssVerbAudioProcessorEditor::setupKnob(KnobWithLabel& knob,
                                                const juce::String& paramId,
                                                const juce::String& labelText,
                                                juce::Colour fillColour)
{
    knob.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    knob.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 65, 16);
    knob.slider.setColour(juce::Slider::rotarySliderFillColourId, fillColour);
    knob.slider.setColour(juce::Slider::rotarySliderOutlineColourId,
                          juce::Colour(0xFF1A2030));
    knob.slider.setColour(juce::Slider::textBoxTextColourId,
                          fillColour.brighter(0.4f));
    knob.slider.setColour(juce::Slider::textBoxOutlineColourId,
                          juce::Colours::transparentBlack);
    knob.slider.setColour(juce::Slider::thumbColourId, fillColour.brighter(0.2f));
    addAndMakeVisible(knob.slider);

    knob.label.setText(labelText, juce::dontSendNotification);
    knob.label.setJustificationType(juce::Justification::centred);
    knob.label.setFont(juce::Font(10.0f));
    knob.label.setColour(juce::Label::textColourId, fillColour.withAlpha(0.7f));
    addAndMakeVisible(knob.label);

    knob.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, paramId, knob.slider);
}

void AbyssVerbAudioProcessorEditor::paint(juce::Graphics& g)
{
    // 深い森の中の古い教会をイメージした背景
    juce::ColourGradient gradient(
        juce::Colour(0xFF0C0F15), 0.0f, 0.0f,
        juce::Colour(0xFF06080C), 0.0f, static_cast<float>(getHeight()),
        false);
    gradient.addColour(0.3, juce::Colour(0xFF0F1520));
    gradient.addColour(0.7, juce::Colour(0xFF0A0D16));
    g.setGradientFill(gradient);
    g.fillAll();

    // 微かな光の粒子
    paintRandom.setSeed(54321);
    for (int i = 0; i < 60; ++i)
    {
        float x = paintRandom.nextFloat() * getWidth();
        float y = paintRandom.nextFloat() * getHeight();
        float alpha = paintRandom.nextFloat() * 0.1f + 0.01f;
        float size = paintRandom.nextFloat() * 1.5f + 0.3f;
        auto color = paintRandom.nextBool()
            ? juce::Colour(0xFFB87A4B) : juce::Colour(0xFF4488AA);
        g.setColour(color.withAlpha(alpha));
        g.fillEllipse(x, y, size, size);
    }

    // タイトル
    g.setColour(juce::Colour(0xFF8EBBCC));
    g.setFont(juce::Font(26.0f, juce::Font::bold));
    g.drawText("A B Y S S V E R B", getLocalBounds().removeFromTop(42),
               juce::Justification::centred);
    g.setFont(juce::Font(11.0f));
    g.setColour(juce::Colour(0xFF6A8899));
    g.drawText("for Violin", getLocalBounds().removeFromTop(58),
               juce::Justification::centred);

    // セクションライン & ラベル
    auto drawSection = [&](float y, const juce::String& text, juce::Colour c)
    {
        g.setColour(c.withAlpha(0.3f));
        g.drawLine(15.0f, y, getWidth() - 15.0f, y, 0.8f);
        g.setColour(c.withAlpha(0.6f));
        g.setFont(juce::Font(11.0f, juce::Font::bold));
        g.drawText(text, 18, static_cast<int>(y) + 2, 250, 16,
                   juce::Justification::centredLeft);
    };

    drawSection(65.0f,  "// VIOLIN INPUT",     juce::Colour(0xFFB87A4B));
    drawSection(195.0f, "// ABYSS REVERB",     juce::Colour(0xFF3A7CA5));
    drawSection(325.0f, "// VANISHING DELAY",  juce::Colour(0xFF6B5B73));
    drawSection(455.0f, "// MIX & EXPRESSION", juce::Colour(0xFF4A9EBF));
}

void AbyssVerbAudioProcessorEditor::resized()
{
    const int kw = 110;  // ノブ幅
    const int kh = 90;   // ノブ高さ
    const int lh = 15;   // ラベル高さ

    auto placeKnob = [&](KnobWithLabel& knob, int x, int y)
    {
        knob.slider.setBounds(x, y, kw, kh);
        knob.label.setBounds(x, y + kh, kw, lh);
    };

    auto centerRow = [&](int numKnobs, int y, auto&&... knobs)
    {
        int totalW = numKnobs * kw + (numKnobs - 1) * 8;
        int startX = (getWidth() - totalW) / 2;
        int idx = 0;
        auto place = [&](KnobWithLabel& k) {
            placeKnob(k, startX + idx * (kw + 8), y);
            idx++;
        };
        (place(knobs), ...);
    };

    // バイオリン入力 (3ノブ)
    centerRow(3, 80, piezoKnob, bodyKnob, brightnessKnob);

    // リバーブ (5ノブ)
    centerRow(5, 210, decayKnob, dampHighKnob, dampLowKnob, shimmerKnob, swayKnob);

    // ディレイ (6ノブ)
    centerRow(6, 340, echoTimeKnob, echoSustainKnob, vanishKnob,
              fadeTexKnob, driftKnob, chorusKnob);

    // ミックス (4ノブ)
    centerRow(4, 470, reverbMixKnob, delayMixKnob, masterMixKnob, bowSensKnob);
}
