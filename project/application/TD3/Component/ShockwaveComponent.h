#pragma once
#include "engine/NoEngine.h"
#include <unordered_set>

/// <summary>
/// 衝撃波コンポーネント
/// 交差点で発生し、円形に拡大してから消滅する
/// </summary>
struct ShockwaveComponent {
	float elapsedTime;						// 経過時間
	float expandDuration;					// 拡大フェーズの時間（秒）
	float fadeDuration;						// フェードアウトの時間（秒）
	float minRadius;						// 最小半径
	float maxRadius;						// 最大半径
	float currentRadius;					// 現在の半径
	float currentAlpha;						// 現在の透明度（1.0 → 0.0）
	
	std::unordered_set<NoEngine::ECS::Entity> hitEnemies;	// 既にヒットした敵のリスト

	ShockwaveComponent()
		: elapsedTime(0.0f),
		expandDuration(0.2f),				// デフォルト: 0.2秒で拡大
		fadeDuration(0.1f),					// デフォルト: 0.1秒でフェード
		minRadius(0.0f),					// デフォルト: 0から開始
		maxRadius(1.0f),					// デフォルト: グリッド1マス分
		currentRadius(0.0f),
		currentAlpha(1.0f) {
	}
};
