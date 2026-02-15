# DroneCosmos — Creative Brief

## Concept
アナログ質感の暴れるドローンシンセサイザー。4基のオシレーターが自己変調・相互変調で
豊かな倍音を生成し、フィードバックFuzzフィルターチェーンで音を彫刻する。
外部アプリ GestureBridge からのOSC入力により、カメラベースのジェスチャーで
リアルタイム操作が可能。

## Sound Character
- クリーンなユニゾンドローンから完全なカオスまで連続的に変化
- FM自己変調によるアナログ的な倍音の暴れ
- 循環相互変調（A→B→C→D→A）によるメタリックなテクスチャ
- フィルターフィードバックの自己発振
- 帰還路Fuzz（GrainCosmos由来の非対称クリッピング）による飽和

## Target Users
サウンドデザイナー、アンビエント/ノイズ/エクスペリメンタル制作者、ライブパフォーマー

## References
- Make Noise 0-Coast（パッチャブルアナログの暴れ感）
- Mutable Instruments Plaits（デジタル変調の多様性）
- Korg MS-20（フィルターの過激さ）
- GrainCosmos（UI/UX、Fuzzエンジン、XYパッドの継承）

## Key Differentiator
- 相互変調マトリクスをマクロパラメータで直感操作
- フィルターフィードバックループ内のFuzz（歪みが蓄積する設計）
- ジェスチャーコントロール（GestureBridge経由OSC）
- XYパッド4面＋残像トレイルエフェクト（GrainCosmosから継承）
