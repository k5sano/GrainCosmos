# Skill: debug-strategy

## Trigger
When build errors or runtime errors fail 2+ times consecutively

## Rules

1. **DO NOT attempt fixes immediately.** First collect and report:
   - Current state of related files
   - Root cause analysis of error messages
   - Verification of dependencies and toolchain (versions, path existence)

2. After gathering information, **present multiple fix strategies** and ask user to choose

3. Implement fixes only after user approves the strategy

## What NOT to do

- Repeatedly edit files on the fly each time an error occurs
- Make assumptions without verification (e.g., path exists, library installed)
- Repeat the same approach 3+ times

---

## Language Variants

# スキル: debug-strategy

## トリガー
ビルドエラーや実行エラーで2回以上連続で失敗したとき

## ルール
1. **即座に修正を試みない**。まず以下の情報を収集して報告する：
   - 関連ファイルの現在の状態
   - エラーメッセージの根本原因の分析
   - 依存関係やツールチェインの確認（バージョン、パスの存在）

2. 情報を集めたら、**修正方針を複数提示**して、ユーザーに選択を求める

3. 修正はユーザーが方針を承認してから実行する

## やってはいけないこと
- エラーが出るたびに場当たり的にファイルを書き換えること
- 確認せずに前提を置くこと（例：パスが存在する、ライブラリがインストール済み）
- 同じアプローチを3回以上繰り返すこと
