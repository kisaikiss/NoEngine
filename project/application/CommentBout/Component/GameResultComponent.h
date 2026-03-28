#pragma once

enum class GameResult { None, Clear, Over };

struct GameResultComponent {
	GameResult result = GameResult::None;

	// Over の理由
	bool playerDied = false;
	bool railReachedEnd = false;

	// デバッグ設定
	bool debugDisableClear = false;  // クリア条件を無効化
	bool debugDisableOver  = false;  // オーバー条件を無効化
};
