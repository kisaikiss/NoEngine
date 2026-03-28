#pragma once
#include "engine/NoEngine.h"
#include "application/CommentBout/Collision/Component/Collider3DComponent.h"

/// <summary>
/// 敵弾の命中判定とダメージ処理を行うシステム。
/// 自機被弾は「スプライト重なり + カメラゲート」共通判定に一本化する。
/// </summary>
class EnemyBulletHitSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;

private:
	bool IsPlayerHitByUnifiedRule(
		No::Registry& registry,
		No::Entity bulletEntity,
		const CommentBoutCollision::Collider3DComponent& bulletCollider,
		No::Entity& outPlayerEntity
	) const;

	bool IsFieldHitByBullet(
		No::Registry& registry,
		No::Entity bulletEntity,
		const CommentBoutCollision::Collider3DComponent& bulletCollider
	) const;
};
