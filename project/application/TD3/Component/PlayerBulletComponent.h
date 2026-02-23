#pragma once
#include "engine/Math/MathInclude.h"

/// <summary>
/// プレイヤーの弾丸コンポーネント
/// 空間上を直進し、一定距離進むか障害物にぶつかると消滅する
/// </summary>
struct PlayerBulletComponent {
	NoEngine::Math::Vector3 direction;	// 進行方向（正規化済み）
	float speed;						// 移動速度
	float travelDistance;				// 移動した距離
	float maxDistance;					// 最大移動距離

	PlayerBulletComponent()
		: direction(NoEngine::Math::Vector3::ZERO),
		speed(5.0f),
		travelDistance(0.0f),
		maxDistance(20.0f) {
	}
};
