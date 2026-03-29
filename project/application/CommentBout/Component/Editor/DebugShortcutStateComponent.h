#pragma once

/// <summary>
/// デバッグショートカットの状態を保持するコンポーネント。
/// DebugShortcutSystem がキー入力を読み取ってフラグを更新する。
/// EditorManager / PlayerControlSystem / GameResultSystem はこのコンポーネントを参照するだけ。
/// </summary>
struct DebugShortcutStateComponent {
    bool useDebugCamera     = false;  // false=RailCamera, true=DebugCamera (LeftShift+1)
    bool debugDisplayAll    = false;  // true=デバッグ表示全展開, false=全折り畳み (LeftShift+2)
    bool debugInvincible    = false;  // 自機無敵 (LeftCtrl+3)
    bool debugDisableResult = false;  // クリア/オーバー判定を両方無効化 (LeftCtrl+4)
};

struct DebugShortcutStateTag {};
