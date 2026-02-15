# DroneCosmos — Architecture

## Signal Flow

OSC A <--> OSC B --. ↕ ✕ ↕ .-> OSC Mix -> SVF Filter -> Comb Filter --> Stutter -> Output Mix -> Limiter -> Out
OSC C <--> OSC D --' ↑           |                                  |
                                    `---- Fuzz ------------------------'
                                    (filter feedback loop)


## Modules

### Module 1: Oscillator Bank（4基）
各オシレーター共通:
- waveform: Sine/Saw/Square/Triangle 連続モーフィング（0.0〜3.0）
- pitch: 粗調整 ±24半音
- detune: 微調整 ±50セント
- level: 0〜100%

変調方式:
- Self-FM: 出力を自分の位相にフィードバック -> 波形が複雑化 -> ノイズへ
- Cross-FM: 隣接ペア変調（A<->B, C<->D）
- Ring-FM: 循環変調（A->B->C->D->A）-> フィードバックFM -> 非線形倍音爆発
- Chaos: 全変調経路にランダム揺らぎ

### Module 2: Filter Chain
1. SVF (State Variable Filter): LP/BP/HP連続モーフィング、自己発振可能
2. Comb Filter: 短ディレイ（0.5〜20ms）+ フィードバック -> 金属的倍音
3. Filter Feedback Loop: フィルター出力->入力の帰還、帰還路にFuzz挿入
   -> ループごとに歪みが蓄積、フィルターレゾナンスピークが歪んで太くなる

### Module 3: Stutter Engine
出力リングバッファからの部分リピート:
- バッファ長可変（10ms〜500ms）
- リピート毎ピッチシフト
- ゲートパターン（8ステップ）
- トリガー: 手動 / 確率 / ジェスチャー（手を握る）

### Module 4: Output
- Limiter（juce::dsp::Limiter）— 安全装置
- Output Volume

### Module 5: OSC Receiver
- juce::OSCReceiver でポート9000リッスン
- GestureBridge からのジェスチャーデータ受信
- マッピングテーブル: ジェスチャー -> パラメータ

## Tech Stack
- JUCE 8 (same submodule as GrainCosmos)
- C++17, CMake + Ninja
- macOS arm64 / VST3 + AU + Standalone
- OSC: juce::OSCReceiver
- License: MIT
