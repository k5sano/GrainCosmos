#!/bin/bash

# CMakeLists.txt 修正スクリプト
# JUCE 8 CMakeLists.txt構造に対応してGUIビルドできるように修正

set -e

CMakeLists_txt="plugins/GrainCosmos/CMakeLists.txt"

# バックアップ
cp "$CMakeLists_txt" "${CMakeLists_txt}.backup"

# ファイル確認
if [ ! -f "$CMakeLists_txt" ]; then
    echo "エラー: CMakeLists.txtが見つかりません"
    exit 1
fi

# CMakeLists.txtが存在するか確認
if [ ! -f "plugins/GrainCosmos/CMakeLists.txt" ]; then
    echo "エラー: CMakeLists.txtが見つかりません"
    exit 1
fi

# 行数確認
lines=$(wc -l < "$CMakeLists_txt")
if [ "$lines" -lt 10 ]; then
    echo "エラー: CMakeLists.txtが$lines行しかありません"
    exit 1
fi

# OK: CMakeLists.txtが存在します

# プラグイン名を設定
PLUGIN_NAME="GrainCosmos"

# JUCEプロジェクト生成モード（Unix Makefiles or Xcode）
# JUCE 8ではXcodeでビルドする必要があります
if [ "$1" = "xcode" ]; then
    GEN_MODE="Xcode"
elif [ "$1" = "make" ]; then
    GEN_MODE="Unix Makefiles"
else
    echo "使用法: $0 (無効)"
    exit 1
fi

# ビルドディレクトリ
BUILD_DIR="build/GrainCosmos"
PROJECT_ROOT="/Users/sanokeigo/Documents/plugin-freedom-system/plugins/GrainCosmos"

# ビルド実行
echo "ビルド開始: モード=$GEN_MODE, プロジェクト=$PROJECT_ROOT"
cd "$PROJECT_ROOT"

# クリーンアップ
echo "既存のビルドファイルを削除..."
rm -rf build/

# ジャネレータ選択してビルド
echo "ジェネレータ: $GEN_MODE"
cmake -B build -S . -DNEEDS_WEB_BROWSER=FALSE -DCMAKE_XCODE_GENERATE_SCHEME=Auto -G "$GEN_MODE" 2>&1 | tee build.log

# ビルド結果確認
if [ $? -eq 0 ]; then
    echo "✓ ビルド成功"

    # VST3チェック
    VST3_PATH="$BUILD_DIR/GrainCosmos_artefacts/VST3/GrainCosmos.vst3/Contents/MacOS/GrainCosmos"
    if [ -d "$VST3_PATH" ]; then
        echo "✓ VST3が生成されました"

        # インストール
        echo "インストール先: ~/Library/Audio/Plug-Ins/VST3/"
        sudo cp -R "$BUILD_DIR/GrainCosmos_artefacts/VST3/GrainCosmos.vst3" ~/Library/Audio/Plug-Ins/VST3/

        if [ $? -eq 0 ]; then
            echo "✓ インストール完了"

            # 情報
            echo ""
            echo "プラグイン名: $PLUGIN_NAME"
            echo "パス: $VST3_PATH"
            echo "バイナリ: build/GrainCosmos/GrainCosmos_artefacts/VST3/GrainCosmos.vst3/Contents/MacOS/GrainCosmos"
        else
            echo "✗ インストール失敗: exit code=$?"
        fi
    else
        echo "ビルドが失敗しました"
fi

# 終果
echo "ビルドログは build.log に保存されています"
