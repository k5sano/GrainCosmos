#pragma once
#include <JuceHeader.h>
#include <random>
#include <cmath>

//==============================================================================
// ピエゾEQ / インプットコンディショナー
// ピエゾ特有の1-3kHzのギスギスを除去 + ボディレゾナンス付加
//==============================================================================
class ViolinInputConditioner
{
public:
    void prepare(double sampleRate)
    {
        sr = sampleRate;
        // ピエゾ補正用ノッチ (2kHz付近)
        calcNotchCoeffs(2200.0, 2.5);
        // ボディレゾナンス用ピーク (440Hz付近、バイオリンの主要共鳴)
        calcResonanceCoeffs(440.0, 3.0, 2.0);
        // ハイシェルフ (明るさ制御)
        calcHighShelfCoeffs(4000.0, 0.0);

        // 状態リセット
        for (int i = 0; i < 3; ++i)
        {
            notchZ1[i] = notchZ2[i] = 0.0f;
            resZ1[i] = resZ2[i] = 0.0f;
            hsZ1[i] = hsZ2[i] = 0.0f;
        }
    }

    void setParameters(float notchDepth, float bodyResonance, float brightness)
    {
        // notchDepth: 0=補正なし, 1=フル補正
        calcNotchCoeffs(2200.0, 2.5 * notchDepth);
        // bodyResonance: 0=なし, 1=豊かなボディ感
        calcResonanceCoeffs(440.0, 3.0, bodyResonance * 4.0);
        // brightness: -6 ~ +6 dB
        calcHighShelfCoeffs(4000.0, (brightness - 0.5f) * 12.0f);
    }

    float process(float input)
    {
        float x = input;
        // ノッチフィルタ（ピエゾ補正）
        x = biquadProcess(x, notchB, notchA, notchZ1, notchZ2);
        // ボディレゾナンス
        x = biquadProcess(x, resB, resA, resZ1, resZ2);
        // ハイシェルフ
        x = biquadProcess(x, hsB, hsA, hsZ1, hsZ2);
        return x;
    }

private:
    double sr = 48000.0;

    // Biquad係数 [b0, b1, b2] / [a0(=1), a1, a2]
    float notchB[3] = {}, notchA[3] = {};
    float resB[3] = {}, resA[3] = {};
    float hsB[3] = {}, hsA[3] = {};

    // Biquad状態
    float notchZ1[3] = {}, notchZ2[3] = {};
    float resZ1[3] = {}, resZ2[3] = {};
    float hsZ1[3] = {}, hsZ2[3] = {};

    float biquadProcess(float x, const float b[3], const float a[3],
                        float z1[3], float z2[3])
    {
        float y = b[0] * x + z1[0];
        z1[0] = b[1] * x - a[1] * y + z2[0];
        z2[0] = b[2] * x - a[2] * y;
        return y;
    }

    void calcNotchCoeffs(double freq, double Q)
    {
        if (Q < 0.01) { notchB[0]=1; notchB[1]=0; notchB[2]=0;
                         notchA[0]=1; notchA[1]=0; notchA[2]=0; return; }
        double w0 = 2.0 * juce::MathConstants<double>::pi * freq / sr;
        double alpha = std::sin(w0) / (2.0 * Q);
        double a0 = 1.0 + alpha;
        notchB[0] = static_cast<float>(1.0 / a0);
        notchB[1] = static_cast<float>(-2.0 * std::cos(w0) / a0);
        notchB[2] = static_cast<float>(1.0 / a0);
        notchA[0] = 1.0f;
        notchA[1] = static_cast<float>(-2.0 * std::cos(w0) / a0);
        notchA[2] = static_cast<float>((1.0 - alpha) / a0);
    }

    void calcResonanceCoeffs(double freq, double Q, double gainDB)
    {
        if (gainDB < 0.01 && gainDB > -0.01) {
            resB[0]=1; resB[1]=0; resB[2]=0;
            resA[0]=1; resA[1]=0; resA[2]=0; return;
        }
        double A = std::pow(10.0, gainDB / 40.0);
        double w0 = 2.0 * juce::MathConstants<double>::pi * freq / sr;
        double alpha = std::sin(w0) / (2.0 * Q);
        double a0 = 1.0 + alpha / A;
        resB[0] = static_cast<float>((1.0 + alpha * A) / a0);
        resB[1] = static_cast<float>((-2.0 * std::cos(w0)) / a0);
        resB[2] = static_cast<float>((1.0 - alpha * A) / a0);
        resA[0] = 1.0f;
        resA[1] = static_cast<float>((-2.0 * std::cos(w0)) / a0);
        resA[2] = static_cast<float>((1.0 - alpha / A) / a0);
    }

    void calcHighShelfCoeffs(double freq, double gainDB)
    {
        if (gainDB < 0.01 && gainDB > -0.01) {
            hsB[0]=1; hsB[1]=0; hsB[2]=0;
            hsA[0]=1; hsA[1]=0; hsA[2]=0; return;
        }
        double A = std::pow(10.0, gainDB / 40.0);
        double w0 = 2.0 * juce::MathConstants<double>::pi * freq / sr;
        double cosw0 = std::cos(w0);
        double alpha = std::sin(w0) / 2.0 * std::sqrt(2.0);
        double sqrtA2alpha = 2.0 * std::sqrt(A) * alpha;
        double a0 = (A+1) - (A-1)*cosw0 + sqrtA2alpha;
        hsB[0] = static_cast<float>(A*((A+1) + (A-1)*cosw0 + sqrtA2alpha) / a0);
        hsB[1] = static_cast<float>(-2.0*A*((A-1) + (A+1)*cosw0) / a0);
        hsB[2] = static_cast<float>(A*((A+1) + (A-1)*cosw0 - sqrtA2alpha) / a0);
        hsA[0] = 1.0f;
        hsA[1] = static_cast<float>(2.0*((A-1) - (A+1)*cosw0) / a0);
        hsA[2] = static_cast<float>(((A+1) - (A-1)*cosw0 - sqrtA2alpha) / a0);
    }
};

//==============================================================================
// エンベロープフォロワー — 弓の強さを追跡
//==============================================================================
class EnvelopeFollower
{
public:
    void prepare(double sampleRate)
    {
        sr = sampleRate;
        envelope = 0.0f;
    }

    void setParameters(float attackMs, float releaseMs)
    {
        attackCoeff  = std::exp(-1.0f / (static_cast<float>(sr) * attackMs  / 1000.0f));
        releaseCoeff = std::exp(-1.0f / (static_cast<float>(sr) * releaseMs / 1000.0f));
    }

    float process(float input)
    {
        float rectified = std::abs(input);
        if (rectified > envelope)
            envelope = attackCoeff * envelope + (1.0f - attackCoeff) * rectified;
        else
            envelope = releaseCoeff * envelope + (1.0f - releaseCoeff) * rectified;
        return envelope;
    }

    float getEnvelope() const { return envelope; }

private:
    double sr = 48000.0;
    float envelope = 0.0f;
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;
};

//==============================================================================
// 深淵リバーブ: 8-line FDN — バイオリン最適化
// 高域の減衰カーブをバイオリンの倍音構造に合わせて調整
//==============================================================================
class AbyssFDNReverb
{
public:
    static constexpr int NUM_LINES = 8;

    void prepare(double sampleRate, int /*samplesPerBlock*/)
    {
        sr = sampleRate;

        // バイオリン用: やや長めのディレイ長で豊かな残響密度
        // 素数ベース、大きな空間をシミュレート
        const int baseLengths[NUM_LINES] = {
            1801, 1913, 1657, 1543, 1381, 1471, 1259, 1163
        };

        for (int i = 0; i < NUM_LINES; ++i)
        {
            int len = static_cast<int>(baseLengths[i] * sr / 44100.0);
            delayLines[i].resize(len, 0.0f);
            writePos[i] = 0;
            dampState[i] = 0.0f;
            // バイオリン用: 2バンドダンピング（低域と高域を別々に制御）
            dampLo[i] = 0.0f;
            dampHi[i] = 0.0f;
        }

        for (int i = 0; i < NUM_LINES; ++i)
            lfoPhase[i] = static_cast<float>(i) / NUM_LINES;
    }

    void setParameters(float decayTime, float dampHigh, float dampLow,
                       float modDepth, float modRate)
    {
        decay = decayTime;
        dampingHigh = dampHigh;
        dampingLow = dampLow;
        this->modDepth = modDepth;
        this->modRate = modRate;
    }

    // envelopeで弓圧に応じてリバーブの広がり方を変える
    float process(float input, float envelope = 0.0f)
    {
        float outputs[NUM_LINES];

        // エンベロープによる動的変調: 強く弾くとモジュレーションが深くなる
        float dynamicMod = modDepth * (1.0f + envelope * 2.0f);

        for (int i = 0; i < NUM_LINES; ++i)
        {
            int len = static_cast<int>(delayLines[i].size());

            lfoPhase[i] += modRate / static_cast<float>(sr);
            if (lfoPhase[i] >= 1.0f) lfoPhase[i] -= 1.0f;

            // 各ラインで異なるLFO波形（sin + 三角波のブレンド）
            float phase = lfoPhase[i];
            float sinLfo = std::sin(2.0f * juce::MathConstants<float>::pi * phase);
            float triLfo = 4.0f * std::abs(phase - 0.5f) - 1.0f;
            float lfo = sinLfo * 0.7f + triLfo * 0.3f;
            float modSamples = lfo * dynamicMod * (static_cast<float>(sr) / 1000.0f);

            // 3次補間読み出し（バイオリンの高域倍音を保つため）
            float readPosF = static_cast<float>(writePos[i])
                           - static_cast<float>(len) + modSamples;
            while (readPosF < 0.0f) readPosF += static_cast<float>(len);

            int idx1 = static_cast<int>(readPosF) % len;
            int idx0 = (idx1 - 1 + len) % len;
            int idx2 = (idx1 + 1) % len;
            int idx3 = (idx1 + 2) % len;
            float frac = readPosF - std::floor(readPosF);

            // Hermite補間
            float y0 = delayLines[i][idx0], y1 = delayLines[i][idx1];
            float y2 = delayLines[i][idx2], y3 = delayLines[i][idx3];
            float c0 = y1;
            float c1 = 0.5f * (y2 - y0);
            float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
            float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
            outputs[i] = ((c3 * frac + c2) * frac + c1) * frac + c0;
        }

        // Hadamardフィードバック
        float feedback[NUM_LINES];
        const float scale = 1.0f / std::sqrt(static_cast<float>(NUM_LINES));
        for (int i = 0; i < NUM_LINES; ++i)
        {
            feedback[i] = 0.0f;
            for (int j = 0; j < NUM_LINES; ++j)
            {
                int bits = i & j;
                int popcount = 0;
                while (bits) { popcount += bits & 1; bits >>= 1; }
                float sign = (popcount % 2 == 0) ? 1.0f : -1.0f;
                feedback[i] += sign * outputs[j];
            }
            feedback[i] *= scale;
        }

        float outputMix = 0.0f;
        for (int i = 0; i < NUM_LINES; ++i)
        {
            int len = static_cast<int>(delayLines[i].size());
            float g = std::pow(10.0f, -3.0f * static_cast<float>(len)
                      / (decay * static_cast<float>(sr)));

            float sig = feedback[i] * g + input / static_cast<float>(NUM_LINES);

            // 2バンド周波数依存ダンピング
            // 高域（バイオリンの倍音がゆっくり消えていく）
            dampHi[i] = sig * (1.0f - dampingHigh) + dampHi[i] * dampingHigh;
            // 低域（深い残響の膨らみを制御）
            float hiPassed = sig - dampHi[i];
            dampLo[i] = hiPassed * (1.0f - dampingLow) + dampLo[i] * dampingLow;
            float processed = dampHi[i] + dampLo[i];

            delayLines[i][writePos[i]] = processed;
            writePos[i] = (writePos[i] + 1) % len;

            outputMix += outputs[i];
        }

        return outputMix * scale;
    }

    void clear()
    {
        for (int i = 0; i < NUM_LINES; ++i)
        {
            std::fill(delayLines[i].begin(), delayLines[i].end(), 0.0f);
            dampState[i] = dampLo[i] = dampHi[i] = 0.0f;
        }
    }

private:
    double sr = 48000.0;
    std::vector<float> delayLines[NUM_LINES];
    int writePos[NUM_LINES] = {};
    float dampState[NUM_LINES] = {};
    float dampLo[NUM_LINES] = {};
    float dampHi[NUM_LINES] = {};
    float lfoPhase[NUM_LINES] = {};

    float decay = 6.0f;
    float dampingHigh = 0.7f;
    float dampingLow = 0.3f;
    float modDepth = 0.5f;
    float modRate = 0.2f;
};

//==============================================================================
// 消失ディレイ — バイオリン版: 弓圧反応 + ピッチドリフト
//==============================================================================
class VanishingDelay
{
public:
    static constexpr int NUM_TAPS = 4; // 4タップ（バイオリンの4弦に呼応するイメージ）

    void prepare(double sampleRate, int /*samplesPerBlock*/)
    {
        sr = sampleRate;
        int maxDelaySamples = static_cast<int>(sr * 3.0); // 最大3秒
        buffer.resize(maxDelaySamples, 0.0f);
        writePos = 0;

        rng.seed(42);
        for (int i = 0; i < NUM_TAPS; ++i)
        {
            tapGainTarget[i] = 1.0f;
            tapGainCurrent[i] = 1.0f;
            tapTimer[i] = 0;
            tapDriftPhase[i] = static_cast<float>(i) * 0.25f;
            degradeLPState[i] = 0.0f;
            // 各タップにわずかなデチューン（合唱効果）
            tapDetunePhase[i] = static_cast<float>(i) * 0.17f;
        }

        // フェードイン/アウト用のクロスフェードバッファ
        prevOutput = 0.0f;
    }

    void setParameters(float delayTimeMs, float feedback, float vanishRate,
                       float degradeAmount, float driftAmount, float detuneAmount)
    {
        this->delayTimeMs = delayTimeMs;
        this->feedback = feedback;
        this->vanishRate = vanishRate;
        this->degradeAmount = degradeAmount;
        this->driftAmount = driftAmount;
        this->detuneAmount = detuneAmount;
    }

    float process(float input, float envelope = 0.0f)
    {
        int bufSize = static_cast<int>(buffer.size());

        // 4タップの間隔 — 5度と4度の音程関係をモチーフにした比率
        const float tapRatios[NUM_TAPS] = { 1.0f, 0.667f, 0.5f, 0.333f };

        float output = 0.0f;

        for (int i = 0; i < NUM_TAPS; ++i)
        {
            // ランダム消失（エンベロープ依存: 弱く弾くと消えやすい）
            tapTimer[i]--;
            if (tapTimer[i] <= 0)
            {
                std::uniform_real_distribution<float> dist(0.0f, 1.0f);
                float roll = dist(rng);

                // 弱音時は消失しやすく、強音時は生き残りやすい
                float effectiveVanishRate = vanishRate * (1.0f - envelope * 0.6f);

                if (roll < effectiveVanishRate)
                {
                    // フェードアウトで消える（バイオリンらしい滑らかさ）
                    tapGainTarget[i] = 0.0f;
                }
                else
                {
                    // 戻る時もフェードイン
                    float newGain = dist(rng) * 0.5f + 0.3f;
                    tapGainTarget[i] = newGain;
                }

                // 次の切り替えタイミング（バイオリンのテンポ感に合わせて長め）
                std::uniform_int_distribution<int> timeDist(
                    static_cast<int>(sr * 0.1),
                    static_cast<int>(sr * 0.8)
                );
                tapTimer[i] = timeDist(rng);
            }

            // なめらかなスムージング（バイオリンの音はブツ切り厳禁）
            float smoothRate = 0.0003f;
            tapGainCurrent[i] += (tapGainTarget[i] - tapGainCurrent[i]) * smoothRate;

            // タイムドリフト + デチューン
            tapDriftPhase[i] += driftAmount * 0.07f / static_cast<float>(sr);
            if (tapDriftPhase[i] >= 1.0f) tapDriftPhase[i] -= 1.0f;
            float drift = std::sin(2.0f * juce::MathConstants<float>::pi * tapDriftPhase[i])
                        * driftAmount * (static_cast<float>(sr) / 1000.0f);

            // 微細ピッチデチューン（コーラス効果 — 弦楽器的な揺らぎ）
            tapDetunePhase[i] += detuneAmount * 0.5f / static_cast<float>(sr);
            if (tapDetunePhase[i] >= 1.0f) tapDetunePhase[i] -= 1.0f;
            float detune = std::sin(2.0f * juce::MathConstants<float>::pi * tapDetunePhase[i])
                         * detuneAmount * 0.3f * (static_cast<float>(sr) / 1000.0f);

            float delaySamples = delayTimeMs * tapRatios[i]
                               * (static_cast<float>(sr) / 1000.0f) + drift + detune;
            delaySamples = juce::jlimit(1.0f, static_cast<float>(bufSize - 4), delaySamples);

            // Hermite補間読み出し
            float readPosF = static_cast<float>(writePos) - delaySamples;
            if (readPosF < 0.0f) readPosF += static_cast<float>(bufSize);
            int idx1 = static_cast<int>(readPosF) % bufSize;
            int idx0 = (idx1 - 1 + bufSize) % bufSize;
            int idx2 = (idx1 + 1) % bufSize;
            int idx3 = (idx1 + 2) % bufSize;
            float frac = readPosF - std::floor(readPosF);

            float y0 = buffer[idx0], y1 = buffer[idx1];
            float y2 = buffer[idx2], y3 = buffer[idx3];
            float c0 = y1;
            float c1 = 0.5f * (y2 - y0);
            float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
            float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
            float tapOut = ((c3 * frac + c2) * frac + c1) * frac + c0;

            // かすれエフェクト: ソフトなローパス劣化（バイオリンなのでビットクラッシュは使わない）
            float lpCoeff = 1.0f - degradeAmount * 0.85f;
            degradeLPState[i] = tapOut * (1.0f - lpCoeff) + degradeLPState[i] * lpCoeff;

            // 劣化量に応じてLP出力とドライをブレンド
            tapOut = tapOut * (1.0f - degradeAmount * 0.7f)
                   + degradeLPState[i] * degradeAmount * 0.7f;

            output += tapOut * tapGainCurrent[i];
        }

        output /= static_cast<float>(NUM_TAPS);

        // フィードバック（高域を少し落としてフィードバックが濁らないように）
        float fbSignal = output * feedback;
        fbLPState = fbSignal * 0.3f + fbLPState * 0.7f;

        buffer[writePos] = input + fbLPState;
        writePos = (writePos + 1) % bufSize;

        prevOutput = output;
        return output;
    }

    void clear()
    {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
        for (int i = 0; i < NUM_TAPS; ++i)
        {
            degradeLPState[i] = 0.0f;
            tapGainCurrent[i] = 1.0f;
            tapGainTarget[i] = 1.0f;
        }
        fbLPState = 0.0f;
        prevOutput = 0.0f;
    }

private:
    double sr = 48000.0;
    std::vector<float> buffer;
    int writePos = 0;

    float delayTimeMs = 500.0f;
    float feedback = 0.5f;
    float vanishRate = 0.3f;
    float degradeAmount = 0.3f;
    float driftAmount = 2.0f;
    float detuneAmount = 1.0f;

    float tapGainTarget[NUM_TAPS] = {};
    float tapGainCurrent[NUM_TAPS] = {};
    int tapTimer[NUM_TAPS] = {};
    float tapDriftPhase[NUM_TAPS] = {};
    float tapDetunePhase[NUM_TAPS] = {};
    float degradeLPState[NUM_TAPS] = {};

    float fbLPState = 0.0f;
    float prevOutput = 0.0f;

    std::mt19937 rng;
};

//==============================================================================
// パラメータースムーサー — クリックノイズを防止
//==============================================================================
struct SmoothedParameters
{
    // バイオリン入力
    float piezoCorrect = 0.6f, bodyResonance = 0.4f, brightness = 0.5f;
    // リバーブ
    float reverbDecay = 8.0f, reverbDampHigh = 0.65f, reverbDampLow = 0.3f;
    float reverbModDepth = 0.6f, reverbModRate = 0.2f;
    // ディレイ
    float delayTime = 500.0f, delayFeedback = 0.45f;
    float vanishRate = 0.25f, degradeAmount = 0.25f;
    float driftAmount = 1.5f, detuneAmount = 1.0f;
    // ミックス
    float reverbMix = 0.45f, delayMix = 0.25f, masterMix = 0.45f;
    float bowSensitivity = 0.5f;

    void reset(float sampleRate)
    {
        // スムージング係数 (約10msのラムプタイム)
        smoothingCoeff = std::exp(-1.0f / (sampleRate * 0.01f));
    }

    void smooth(const float* rawTargets)
    {
        // 0-18番目のパラメーターを順にスムージング
        float* targets[] = {
            &piezoCorrect, &bodyResonance, &brightness,
            &reverbDecay, &reverbDampHigh, &reverbDampLow, &reverbModDepth, &reverbModRate,
            &delayTime, &delayFeedback, &vanishRate, &degradeAmount, &driftAmount, &detuneAmount,
            &reverbMix, &delayMix, &masterMix, &bowSensitivity
        };

        for (size_t i = 0; i < 18; ++i)
        {
            *targets[i] += (rawTargets[i] - *targets[i]) * (1.0f - smoothingCoeff);
        }
    }

private:
    float smoothingCoeff = 0.999f;
};

//==============================================================================
// メインプロセッサ
//==============================================================================
class AbyssVerbAudioProcessor : public juce::AudioProcessor
{
public:
    AbyssVerbAudioProcessor();
    ~AbyssVerbAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // パラメータースムージング
    SmoothedParameters smoothed;
    float rawParamBuffer[18]; // 生パラメーターの一時バッファ

    // ステレオペア
    ViolinInputConditioner conditionerL, conditionerR;
    EnvelopeFollower envFollowerL, envFollowerR;
    AbyssFDNReverb reverbL, reverbR;
    VanishingDelay delayL, delayR;

    // DCブロッカー
    float dcBlockL_x1 = 0.0f, dcBlockL_y1 = 0.0f;
    float dcBlockR_x1 = 0.0f, dcBlockR_y1 = 0.0f;

    // ソフトリミッター用
    float softClip(float x)
    {
        if (x > 1.0f) return 1.0f - std::exp(-(x - 1.0f));
        if (x < -1.0f) return -(1.0f - std::exp(-(-x - 1.0f)));
        return x;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AbyssVerbAudioProcessor)
};
