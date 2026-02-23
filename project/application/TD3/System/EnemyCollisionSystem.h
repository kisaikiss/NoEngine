#pragma once
#include "engine/NoEngine.h"
#include "../Component/HealthComponent.h"

/// <summary>
/// 敵衝突システム
/// 敵 ↔ プレイヤーのワールド座標近傍判定を行い、
/// 接触時に敵を死亡させ、プレイヤーに1ダメージを与える。
///
/// 弾 → 敵 の衝突は PlayerBulletSystem が担当する。
/// </summary>
class EnemyCollisionSystem : public No::ISystem {
public:
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(No::Registry& registry, float deltaTime) override;
private:
	// 衝突判定距離（二乗で管理してsqrt省略）
	static constexpr float COLLISION_RADIUS = 0.4f;
	static constexpr float COLLISION_RADIUS_SQ = COLLISION_RADIUS * COLLISION_RADIUS;

#ifdef USE_IMGUI
	/// <summary>
	/// プレイヤーの HP 表示・無敵フラグ操作ウィンドウ
	/// </summary>
	void DebugUI(HealthComponent* playerHealth);
#endif
};