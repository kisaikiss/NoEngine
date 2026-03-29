#pragma once

/// <summary>
/// デバッグショートカットの状態を保持するコンポーネント。
/// DebugShortcutSystem がキー入力を読み取ってフラグを更新する。
/// EditorManager / PlayerControlSystem / GameResultSystem はこのコンポーネントを参照するだけ。
/// </summary>
struct DebugShortcutStateComponent {
	bool useDebugCamera = false;  // false=RailCamera, true=DebugCamera (LeftShift+1)
	bool debugDisplayAll = false;  // true=デバッグ表示項目を全ON, false=全OFF (LeftShift+2)
	bool applyDebugDisplayAll = false; // trueのフレームで一括適用し、適用側でfalseに戻す
	bool debugInvincible = false;  // 自機無敵 (LeftShift+3)
	bool debugDisableResult = false;  // クリア/オーバー判定を両方無効化 (LeftShift+4)
};

struct DebugShortcutStateTag {};
