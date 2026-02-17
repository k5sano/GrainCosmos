#include "PluginProcessor.h"
#include "PluginEditor.h"

AbyssVerbAudioProcessor::AbyssVerbAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

AbyssVerbAudioProcessor::~AbyssVerbAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout
AbyssVerbAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // === バイオリン入力調整 ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"piezoCorrect", 1}, "Piezo Correction",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.6f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"bodyResonance", 1}, "Body Resonance",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.4f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"brightness", 1}, "Brightness",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));

    // === リバーブ ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"reverbDecay", 1}, "Abyss Depth",
        juce::NormalisableRange<float>(0.5f, 45.0f, 0.1f, 0.35f), 8.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"reverbDampHigh", 1}, "High Darkness",
        juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f), 0.65f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"reverbDampLow", 1}, "Low Warmth",
        juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f), 0.3f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"reverbModDepth", 1}, "Shimmer",
        juce::NormalisableRange<float>(0.0f, 3.0f, 0.01f), 0.6f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"reverbModRate", 1}, "Sway",
        juce::NormalisableRange<float>(0.03f, 1.5f, 0.01f), 0.2f));

    // === ディレイ ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"delayTime", 1}, "Echo Time",
        juce::NormalisableRange<float>(80.0f, 2000.0f, 1.0f, 0.45f), 500.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"delayFeedback", 1}, "Echo Sustain",
        juce::NormalisableRange<float>(0.0f, 0.92f, 0.01f), 0.45f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"vanishRate", 1}, "Vanish",
        juce::NormalisableRange<float>(0.0f, 0.7f, 0.01f), 0.25f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"degradeAmount", 1}, "Fade Texture",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.25f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"driftAmount", 1}, "Time Drift",
        juce::NormalisableRange<float>(0.0f, 8.0f, 0.1f), 1.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"detuneAmount", 1}, "Chorus Drift",
        juce::NormalisableRange<float>(0.0f, 5.0f, 0.1f), 1.0f));

    // === ミックス ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"reverbMix", 1}, "Abyss Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.45f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"delayMix", 1}, "Echo Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.25f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"masterMix", 1}, "Dry/Wet",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.45f));

    // === 表現力 ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"bowSensitivity", 1}, "Bow Sensitivity",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));

    return { params.begin(), params.end() };
}

//==============================================================================
void AbyssVerbAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    conditionerL.prepare(sampleRate);
    conditionerR.prepare(sampleRate);

    envFollowerL.prepare(sampleRate);
    envFollowerR.prepare(sampleRate);
    envFollowerL.setParameters(5.0f, 150.0f);  // 弓の速いアタック、ゆっくりリリース
    envFollowerR.setParameters(5.0f, 150.0f);

    reverbL.prepare(sampleRate, samplesPerBlock);
    reverbR.prepare(sampleRate, samplesPerBlock);
    delayL.prepare(sampleRate, samplesPerBlock);
    delayR.prepare(sampleRate, samplesPerBlock);

    reverbL.clear();
    reverbR.clear();
    delayL.clear();
    delayR.clear();

    // スムーザー初期化
    smoothed.reset(static_cast<float>(sampleRate));

    // 現在のパラメーター値でスムーザーを初期化
    rawParamBuffer[0]  = apvts.getRawParameterValue("piezoCorrect")->load();
    rawParamBuffer[1]  = apvts.getRawParameterValue("bodyResonance")->load();
    rawParamBuffer[2]  = apvts.getRawParameterValue("brightness")->load();
    rawParamBuffer[3]  = apvts.getRawParameterValue("reverbDecay")->load();
    rawParamBuffer[4]  = apvts.getRawParameterValue("reverbDampHigh")->load();
    rawParamBuffer[5]  = apvts.getRawParameterValue("reverbDampLow")->load();
    rawParamBuffer[6]  = apvts.getRawParameterValue("reverbModDepth")->load();
    rawParamBuffer[7]  = apvts.getRawParameterValue("reverbModRate")->load();
    rawParamBuffer[8]  = apvts.getRawParameterValue("delayTime")->load();
    rawParamBuffer[9]  = apvts.getRawParameterValue("delayFeedback")->load();
    rawParamBuffer[10] = apvts.getRawParameterValue("vanishRate")->load();
    rawParamBuffer[11] = apvts.getRawParameterValue("degradeAmount")->load();
    rawParamBuffer[12] = apvts.getRawParameterValue("driftAmount")->load();
    rawParamBuffer[13] = apvts.getRawParameterValue("detuneAmount")->load();
    rawParamBuffer[14] = apvts.getRawParameterValue("reverbMix")->load();
    rawParamBuffer[15] = apvts.getRawParameterValue("delayMix")->load();
    rawParamBuffer[16] = apvts.getRawParameterValue("masterMix")->load();
    rawParamBuffer[17] = apvts.getRawParameterValue("bowSensitivity")->load();
    smoothed.smooth(rawParamBuffer);

    dcBlockL_x1 = dcBlockL_y1 = 0.0f;
    dcBlockR_x1 = dcBlockR_y1 = 0.0f;
}

void AbyssVerbAudioProcessor::releaseResources() {}

bool AbyssVerbAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    auto mainOut = layouts.getMainOutputChannelSet();
    auto mainIn = layouts.getMainInputChannelSet();

    // ステレオまたはモノ入力 → ステレオ出力
    if (mainOut != juce::AudioChannelSet::stereo())
        return false;
    if (mainIn != juce::AudioChannelSet::stereo()
        && mainIn != juce::AudioChannelSet::mono())
        return false;
    return true;
}

void AbyssVerbAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                            juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // 生パラメーターをバッファに取得
    rawParamBuffer[0]  = apvts.getRawParameterValue("piezoCorrect")->load();
    rawParamBuffer[1]  = apvts.getRawParameterValue("bodyResonance")->load();
    rawParamBuffer[2]  = apvts.getRawParameterValue("brightness")->load();
    rawParamBuffer[3]  = apvts.getRawParameterValue("reverbDecay")->load();
    rawParamBuffer[4]  = apvts.getRawParameterValue("reverbDampHigh")->load();
    rawParamBuffer[5]  = apvts.getRawParameterValue("reverbDampLow")->load();
    rawParamBuffer[6]  = apvts.getRawParameterValue("reverbModDepth")->load();
    rawParamBuffer[7]  = apvts.getRawParameterValue("reverbModRate")->load();
    rawParamBuffer[8]  = apvts.getRawParameterValue("delayTime")->load();
    rawParamBuffer[9]  = apvts.getRawParameterValue("delayFeedback")->load();
    rawParamBuffer[10] = apvts.getRawParameterValue("vanishRate")->load();
    rawParamBuffer[11] = apvts.getRawParameterValue("degradeAmount")->load();
    rawParamBuffer[12] = apvts.getRawParameterValue("driftAmount")->load();
    rawParamBuffer[13] = apvts.getRawParameterValue("detuneAmount")->load();
    rawParamBuffer[14] = apvts.getRawParameterValue("reverbMix")->load();
    rawParamBuffer[15] = apvts.getRawParameterValue("delayMix")->load();
    rawParamBuffer[16] = apvts.getRawParameterValue("masterMix")->load();
    rawParamBuffer[17] = apvts.getRawParameterValue("bowSensitivity")->load();

    // モノ入力対応
    auto* channelL = buffer.getWritePointer(0);
    auto* channelR = buffer.getWritePointer(totalNumInputChannels > 1 ? 1 : 0);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        // パラメータースムージング（サンプルごとに更新）
        smoothed.smooth(rawParamBuffer);

        // スムージングされたパラメーターを使用
        float piezoCorrect   = smoothed.piezoCorrect;
        float bodyResonance  = smoothed.bodyResonance;
        float brightness     = smoothed.brightness;
        float reverbDecay    = smoothed.reverbDecay;
        float reverbDampHigh = smoothed.reverbDampHigh;
        float reverbDampLow  = smoothed.reverbDampLow;
        float reverbModDepth = smoothed.reverbModDepth;
        float reverbModRate  = smoothed.reverbModRate;
        float delayTime      = smoothed.delayTime;
        float delayFeedback  = smoothed.delayFeedback;
        float vanishRate     = smoothed.vanishRate;
        float degradeAmount  = smoothed.degradeAmount;
        float driftAmount    = smoothed.driftAmount;
        float detuneAmount   = smoothed.detuneAmount;
        float reverbMix      = smoothed.reverbMix;
        float delayMix       = smoothed.delayMix;
        float masterMix      = smoothed.masterMix;
        float bowSensitivity = smoothed.bowSensitivity;

        // パラメータ設定
        conditionerL.setParameters(piezoCorrect, bodyResonance, brightness);
        conditionerR.setParameters(piezoCorrect, bodyResonance, brightness);

        reverbL.setParameters(reverbDecay, reverbDampHigh, reverbDampLow,
                              reverbModDepth, reverbModRate);
        reverbR.setParameters(reverbDecay, reverbDampHigh, reverbDampLow,
                              reverbModDepth, reverbModRate);

        delayL.setParameters(delayTime, delayFeedback, vanishRate,
                             degradeAmount, driftAmount, detuneAmount);
        // R側をわずかにずらす → ステレオ幅
        delayR.setParameters(delayTime * 1.05f, delayFeedback, vanishRate,
                             degradeAmount, driftAmount * 1.12f, detuneAmount * 0.9f);

        // === 入力調整 ===
        float dryL = conditionerL.process(channelL[sample]);
        float dryR = conditionerR.process(channelR[sample]);

        // === エンベロープ追跡 ===
        float envL = envFollowerL.process(dryL);
        float envR = envFollowerR.process(dryR);
        // 弓圧感度の適用
        float bowEnvL = envL * bowSensitivity * 3.0f;
        float bowEnvR = envR * bowSensitivity * 3.0f;
        bowEnvL = juce::jlimit(0.0f, 1.0f, bowEnvL);
        bowEnvR = juce::jlimit(0.0f, 1.0f, bowEnvR);

        // === ディレイ（弓圧反応付き） ===
        float delOutL = delayL.process(dryL, bowEnvL);
        float delOutR = delayR.process(dryR, bowEnvR);

        // === リバーブ（ドライ + ディレイを混ぜて入力） ===
        float reverbInL = dryL + delOutL * delayMix * 0.7f;
        float reverbInR = dryR + delOutR * delayMix * 0.7f;

        float revOutL = reverbL.process(reverbInL, bowEnvL);
        float revOutR = reverbR.process(reverbInR, bowEnvR);

        // === ウェット信号合成 ===
        float wetL = revOutL * reverbMix + delOutL * delayMix;
        float wetR = revOutR * reverbMix + delOutR * delayMix;

        // === DCブロッカー ===
        const float dcCoeff = 0.9975f;
        float dcOutL = wetL - dcBlockL_x1 + dcCoeff * dcBlockL_y1;
        dcBlockL_x1 = wetL;
        dcBlockL_y1 = dcOutL;
        wetL = dcOutL;

        float dcOutR = wetR - dcBlockR_x1 + dcCoeff * dcBlockR_y1;
        dcBlockR_x1 = wetR;
        dcBlockR_y1 = dcOutR;
        wetR = dcOutR;

        // === ソフトリミッター（バイオリンの音をクリップさせない） ===
        wetL = softClip(wetL);
        wetR = softClip(wetR);

        // === ドライ/ウェットミックス ===
        channelL[sample] = dryL * (1.0f - masterMix) + wetL * masterMix;
        channelR[sample] = dryR * (1.0f - masterMix) + wetR * masterMix;
    }
}

//==============================================================================
juce::AudioProcessorEditor* AbyssVerbAudioProcessor::createEditor()
{
    return new AbyssVerbAudioProcessorEditor(*this);
}

bool AbyssVerbAudioProcessor::hasEditor() const { return true; }
const juce::String AbyssVerbAudioProcessor::getName() const { return JucePlugin_Name; }
bool AbyssVerbAudioProcessor::acceptsMidi() const { return false; }
bool AbyssVerbAudioProcessor::producesMidi() const { return false; }
bool AbyssVerbAudioProcessor::isMidiEffect() const { return false; }
double AbyssVerbAudioProcessor::getTailLengthSeconds() const { return 15.0; }
int AbyssVerbAudioProcessor::getNumPrograms() { return 1; }
int AbyssVerbAudioProcessor::getCurrentProgram() { return 0; }
void AbyssVerbAudioProcessor::setCurrentProgram(int) {}
const juce::String AbyssVerbAudioProcessor::getProgramName(int) { return {}; }
void AbyssVerbAudioProcessor::changeProgramName(int, const juce::String&) {}

void AbyssVerbAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AbyssVerbAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AbyssVerbAudioProcessor();
}
