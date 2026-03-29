#pragma once

enum class GameResult { None, Clear, Over };

struct GameResultComponent {
	GameResult result = GameResult::None;

	// Over の理由
	bool playerDied = false;
	bool railReachedEnd = false;
	// デバッグ無効フラグは DebugShortcutStateComponent::debugDisableResult に移植済み
};
