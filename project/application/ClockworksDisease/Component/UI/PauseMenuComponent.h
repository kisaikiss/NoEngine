#pragma once
#include "engine/NoEngine.h"

enum class PauseMenuState {
	kClosed, // 非表示
	kMain,   // Resume/Option/タイトルへ戻る/ゲームをやめる の選択画面
	kOption, // オプション画面
};

// ポーズメニュー全体の状態を保持する。PauseMenuSystemが初回Updateで生成する
// 専用の管理Entityに付ける。
struct PauseMenuComponent {
	PauseMenuState state = PauseMenuState::kClosed;
	int mainSelectedIndex = 0;   // 0:Resume 1:Option 2:タイトルへ戻る 3:ゲームをやめる
	int optionSelectedIndex = 0; // オプション項目のインデックス（現状はカメラ移動速度のみ）
};

// メインメニュー各項目のテキストEntityに付けるタグ（デバッグ識別用）
struct PauseMenuItemTag {
	int index = 0;
};