# DroneCosmos — Parameter Specification

## Oscillator Bank (16 params)

| ID | Name | Range | Default | Unit |
|----|------|-------|---------|------|
| osc_a_waveform | OSC A Wave | 0.0 - 3.0 | 0.0 (Sine) | — |
| osc_a_pitch | OSC A Pitch | -24 - +24 | 0 | semitones |
| osc_a_detune | OSC A Detune | -50 - +50 | 0 | cents |
| osc_a_level | OSC A Level | 0 - 100 | 75 | % |
| osc_b_waveform | OSC B Wave | 0.0 - 3.0 | 0.0 | — |
| osc_b_pitch | OSC B Pitch | -24 - +24 | 0 | semitones |
| osc_b_detune | OSC B Detune | -50 - +50 | 7 | cents |
| osc_b_level | OSC B Level | 0 - 100 | 75 | % |
| osc_c_waveform | OSC C Wave | 0.0 - 3.0 | 1.0 (Saw) | — |
| osc_c_pitch | OSC C Pitch | -24 - +24 | -12 | semitones |
| osc_c_detune | OSC C Detune | -50 - +50 | -5 | cents |
| osc_c_level | OSC C Level | 0 - 100 | 50 | % |
| osc_d_waveform | OSC D Wave | 0.0 - 3.0 | 2.0 (Square) | — |
| osc_d_pitch | OSC D Pitch | -24 - +24 | 7 | semitones |
| osc_d_detune | OSC D Detune | -50 - +50 | 3 | cents |
| osc_d_level | OSC D Level | 0 - 100 | 50 | % |

Default配置: A=Sine(root), B=Sine(+7cent), C=Saw(-1oct,-5cent), D=Square(+5th,+3cent)
-> 起動時から豊かなドローンが鳴る

## Modulation Matrix (10 params)

| ID | Name | Range | Default | Description |
|----|------|-------|---------|-------------|
| mod_range | Modulation Range | Low/Mid/High | Mid | L=繊細/M=バランス/H=過激 |
| mod_mode | Modulation Mode | 0.0 - 1.0 | 0.0 | 0.0=Drone(穏やか) / 1.0=Noise(過激) |
| self_fb_phase | Self FB Phase | 0 - 100 | 0 | 位相フィードバック（FM的） |
| self_fb_amp | Self FB Amplitude | 0 - 100 | 0 | 振幅フィードバック（AM的） |
| self_fb_delay | Self FB Delay | 0 - 100 | 0 | ディレイフィードバック（コム的） |
| self_fb_delay_time | Self FB Delay Time | 0.1 - 50.0 | 5.0 | ms |
| self_fb_pitch | Self FB Pitch Shift | -24 - +24 | 0 | semitones |
| cross_mod | Cross Modulation | 0 - 100 | 0 | A<->B, C<->D相互FM |
| ring_mod | Ring Modulation | 0 - 100 | 0 | A->B->C->D->A循環FM |
| chaos_mod | Chaos Modulation | 0 - 100 | 0 | 変調のランダム揺らぎ |

### Self Feedback System

**self_fb_phase (Phase Feedback)**:
- 出力を自分の位相増分にフィードバック
- FM変調的：周波数が動的に変化
- 倍音の生成、非線形なスペクトル変化

**self_fb_amp (Amplitude Feedback)**:
- 出力を自分の振幅にフィードバック
- AM変調的：音量が出力値に応じて変動
- トレモロ〜リングモッド的な効果
- 実装: `output *= (1.0 + ampScale * prevOutput)`

**self_fb_delay (Delay Feedback)**:
- 出力を短いディレイバッファ経由で位相に戻す
- ディレイ時間で帰還の音程が変わる
- カルンギ/コムフィルター的な共鳴効果
- バッファサイズ：最大50ms (2400サンプル@48kHz)

**self_fb_pitch (Pitch Shift in Feedback)**:
- フィードバック信号をピッチシフトしてから戻す
- +12で1オクターブ上の信号が戻る（シマー的効果）
- -12で1オクターブ下（サブオシレーター的効果）
- 実装：リングバッファの読み出しレートを変更

### Modulation Range (mod_range)
**Low (繊細なドローン向け)**:
- self_fb_*: 0-100 → 変調指数 0-0.1
- cross_mod: 0-100 → 変調指数 0-0.2
- ring_mod: 0-100 → 変調指数 0-0.2
- chaos_mod: 0-100 → LFO振幅 0-0.05
- 用途: 繊細なテクスチャ、環境音楽的なドローン

**Mid (バランス)**:
- self_fb_*: 0-100 → 変調指数 0-1.0
- cross_mod: 0-100 → 変調指数 0-2.0
- ring_mod: 0-100 → 変調指数 0-2.0
- chaos_mod: 0-100 → LFO振幅 0-0.3
- 用途: 汎用的なドローンシンセサイザー

**High (ノイズマシン)**:
- self_fb_*: 0-100 → 変調指数 0-5.0
- cross_mod: 0-100 → 変調指数 0-10.0
- ring_mod: 0-100 → 変調指数 0-10.0
- chaos_mod: 0-100 → LFO振幅 0-2.0
- 用途: 激しいテクスチャ、ノイズ、サウンドデザイン

### Modulation Mode Scaling
mod_mode は mod_range で設定された範囲内でさらに Drone/Noise の補間を行います。

**Drone Mode (mod_mode = 0.0)**:
- mod_range の範囲を 0.3-0.5 倍に抑制

**Noise Mode (mod_mode = 1.0)**:
- mod_range の範囲を 5-8 倍に拡大

## Filter Chain (7 params)

| ID | Name | Range | Default | Description |
|----|------|-------|---------|-------------|
| cutoff | Filter Cutoff | 20 - 20000 | 1000 | Hz, 対数スケール |
| resonance | Filter Resonance | 0 - 100 | 0 | 自己発振可能 |
| filter_morph | Filter Type | 0 - 100 | 0 | LP(0)->BP(50)->HP(100) |
| comb_time | Comb Time | 0.5 - 20.0 | 5.0 | ms |
| comb_feedback | Comb Feedback | 0 - 99 | 0 | % |
| filter_feedback | Filter FB Amount | 0 - 100 | 0 | フィルター帰還量 |
| filter_fuzz | Filter Fuzz | 0 - 100 | 0 | 帰還路の非対称Fuzz量 |

## Stutter Engine (5 params)

| ID | Name | Range | Default | Description |
|----|------|-------|---------|-------------|
| stutter_length | Stutter Length | 10 - 500 | 100 | ms |
| stutter_repeats | Stutter Repeats | 1 - 32 | 4 | 回数 |
| stutter_pitch | Stutter Pitch | -12 - +12 | 0 | リピート毎半音シフト |
| stutter_gate | Stutter Gate | 0 - 255 | 255 | 8bit ゲートパターン |
| stutter_trigger | Stutter Trigger | 0 - 100 | 0 | 発動確率 (0=OFF) |

## Output (4 params)

| ID | Name | Range | Default | Description |
|----|------|-------|---------|-------------|
| output_volume | Output Volume | 0 - 100 | 80 | % |
| limiter_threshold | Limiter Threshold | -20 - 0 | -1 | dB |
| limiter_release | Limiter Release | 10 - 500 | 100 | ms |
| drone_pitch | Drone Base Pitch | 20 - 500 | 55 | Hz (A1=55Hz) |

## OSC Settings (non-automatable)

| ID | Name | Default | Description |
|----|------|---------|-------------|
| osc_port | OSC Port | 9000 | 受信ポート |
| osc_enabled | OSC Enable | OFF | ON/OFF |

## Total: 43 automatable parameters + 2 settings

## Gesture Mapping Defaults

| Gesture | Parameter | Description |
|---------|-----------|-------------|
| 手の開閉 | cutoff | 閉じる=低、開く=高 |
| 手の高さ (Y) | self_mod | 上=暴れる |
| 手の横位置 (X) | filter_morph | 左=LP、右=HP |
| 両手の距離 | resonance | 近い=低、離す=高 |
| 指の本数 | stutter_repeats | 握り拳=スタッター発動 |
| 手首の回転 | cross_mod | 回すほど変調増加 |
