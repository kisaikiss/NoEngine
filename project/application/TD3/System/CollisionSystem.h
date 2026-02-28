#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// 衝突判定システム
/// 全てのSphereColliderComponentを総当たりでチェックし、
/// 衝突フラグとエンティティ情報を設定する。
/// 実際のダメージ処理は他のSystemで行う。
/// </summary>
class CollisionSystem : public No::ISystem {
public:
	/// <summary>
	/// 更新処理
	/// 1. UpdateCollider: 全コライダーのワールド座標・半径を更新
	/// 2. CheckSphereToSphere: 総当たりで衝突判定
	/// </summary>
	void Update(No::Registry& registry, float deltaTime) override;

private:
	/// <summary>
	/// 全コライダーのワールド座標と半径を更新し、衝突フラグをリセット
	/// </summary>
	void UpdateCollider(No::Registry& registry);

	/// <summary>
	/// 球体同士の衝突判定（距離ベース）
	/// </summary>
	static bool CheckSphereToSphere(const No::Vector3& center1, const No::Vector3& center2, const float radius1, const float radius2);

	/// <summary>
	/// ボックス同士の衝突判定（AABB、将来の拡張用）
	/// </summary>
	static bool CheckBoxToBox(const No::Vector3& center1, const No::Vector3& center2, const No::Vector3& size1, const No::Vector3& size2);

	/// <summary>
	/// ボックスと球体の衝突判定（将来の拡張用）
	/// </summary>
	static bool CheckBoxToSphere(const No::Vector3& center1, const No::Vector3& center2, const No::Vector3& size, const float radius);
};
