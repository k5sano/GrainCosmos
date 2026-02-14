
FPS のようにお手本プラグインをリポジトリに含めて Claude Code に参照させる、という使い方に向いたオープンソースのリソースはかなり充実しています。

一番の宝庫は **[awesome-juce](https://github.com/sudara/awesome-juce)** です。JUCE で作られたオープンソースのプラグインがカテゴリ別に整理されていて、更新頻度のステータスも毎晩自動更新されています。ここからお手本を選ぶのが最も効率的です。

特に Claude Code のお手本として使いやすそうなものをいくつか挙げると、まずシンセ系では **Surge**（surge-synthesizer/surge）が圧倒的に活発で、モジュレーションマトリクスやウェーブテーブルなど高度な実装の参考になります。もう少しコンパクトなものなら **Odin2**（TheWaveWarden/odin2）が24ボイスポリフォニックシンセで、コードの規模感がちょうどいい。**Vital**（mtytel/vital）はウェーブテーブルシンセの実装としては最高峰ですが、コードベースが大きいのでピンポイントで参照する形になります。

エフェクト系では **Chowdhury-DSP** のプラグイン群が非常に質が高いです。**AnalogTapeModel** はテープシミュレーション、**BYOD** はギターディストーション、**ChowMatrix** はディレイツリーで、どれもDSPのコードがきれいに書かれています。コンプレッサーなら **valentine**（tote-bag-labs/valentine）や **CTAGDRC** がシンプルで参考にしやすい。

グラニュラー系で言えば **paulxstretch**（essej/paulxstretch）がタイムストレッチの実装として面白いし、GrainCosmos の方向性にも近いです。

テンプレートとしては **pamplejuce**（sudara/pamplejuce）が秀逸で、CMake + Catch2 + pluginval の GitHub Actions が最初から組み込まれています。FPS の CI 構成を強化するなら参考になるはずです。

UIモジュールでは **foleys_gui_magic** が WYSIWYG でプラグイン GUI を作れるので、Stage 3 の GUI 実装でも使えるかもしれません。

実用的な使い方としては、FPS の中にお手本用のディレクトリを作って、たとえばエフェクトを作るときは BYOD や valentine のソースを、シンセを作るときは Odin2 のソースを参照ファイルとして置いておく、という形が一番 Claude Code に効きそうです。動くコードが一番雄弁なドキュメントですからね。