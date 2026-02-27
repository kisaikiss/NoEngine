#pragma once
#include "engine/Math/MathInclude.h"
#include <unordered_set>

/// <summary>
/// プレイヤーの弾丸コンポーネント
/// 空間上を直進し、グリッド上で進行できない地点に到達すると消滅する
/// </summary>
struct PlayerBulletComponent {
	NoEngine::Math::Vector3 direction;	// 進行方向（正規化済み）
	int startNodeX;						// 発射元のグリッドX座標（このノードのみグリッド判定をスキップする）
	int startNodeY;						// 発射元のグリッドY座標
	float speed;						// 移動速度
	float travelDistance;				// 移動した距離
	float maxDistance;					// 最大移動距離（安全網として保持しておく）
	
	std::unordered_set<int> visitedIntersections;	// 訪問済み交差点（重複発生防止用）

	PlayerBulletComponent()
		: direction(NoEngine::Math::Vector3::ZERO),
		startNodeX(0),
		startNodeY(0),
		speed(5.0f),
		travelDistance(0.0f),
		maxDistance(20.0f) {
	}
};