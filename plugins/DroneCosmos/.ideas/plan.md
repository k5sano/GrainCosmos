# DroneCosmos — Implementation Plan

## Phase 1: Sound Engine（オシレーター＋変調）
- [ ] プロジェクトスキャフォールド（CMakeLists.txt, PluginProcessor, PluginEditor）
- [ ] オシレーター4基実装（波形モーフィング: sin->saw->square->tri）
- [ ] ピッチ、デチューン、レベル
- [ ] drone_pitch パラメータ（MIDIなしでドローンが鳴る）
- [ ] 自己変調（self_mod）実装
- [ ] 相互変調（cross_mod: A<->B, C<->D）実装
- [ ] 循環変調（ring_mod: A->B->C->D->A）実装
- [ ] 変調揺らぎ（chaos_mod）実装
- [ ] ビルド＆テスト: パラメータ操作で音が暴れることを確認

## Phase 2: Filter Chain
- [ ] SVF実装（cutoff, resonance, filter_morph: LP/BP/HP）
- [ ] 自己発振の確認（resonance > 90%で発振）
- [ ] コムフィルター実装（comb_time, comb_feedback）
- [ ] フィルターフィードバックループ実装（filter_feedback）
- [ ] 帰還路Fuzz実装（filter_fuzz）— GrainCosmosのapplyDistortion移植
- [ ] ビルド＆テスト: フィルター操作でドローンが歌うことを確認

## Phase 3: Stutter + Output
- [ ] スタッターエンジン実装（リングバッファ、リピート、ピッチシフト）
- [ ] ゲートパターン実装
- [ ] リミッター実装（juce::dsp::Limiter）
- [ ] Output Volume
- [ ] ビルド＆テスト

## Phase 4: UI
- [ ] GrainCosmosからXYPadComponent.hをコピー＆調整
- [ ] ヘッダー（タイトル、プリセット、主要スライダー）
- [ ] 残像トレイルエフェクト、背景画像（GrainCosmosと同機能）
- [ ] プリセットシステム（JSON保存/読み込み）
- [ ] OSC接続状態表示

## Phase 5: OSC Integration
- [ ] juce::OSCReceiver 実装
- [ ] /gesture/hand/0/* アドレスの受信＆パース
- [ ] ジェスチャー->パラメータマッピングテーブル
- [ ] GestureBridge との接続テスト

## GestureBridge（別リポジトリ）
- [ ] Python + MediaPipe + python-osc
- [ ] カメラ起動、手のランドマーク追跡
- [ ] OSCメッセージ送信
- [ ] 簡易GUI（手の状態表示、送信先設定）

## Notes
- GrainCosmosのソースを参照: ~/Documents/plugin-freedom-system/plugins/GrainCosmos/
- applyDistortion(), XYPadComponent.h, preset system は流用可能
- ドローンシンセなのでMIDI入力不要、drone_pitchパラメータで基音を決定
- output_mix (dry/wet) はシンセなので不要、常にウェット100%
