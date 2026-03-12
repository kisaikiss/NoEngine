#pragma once
#include "engine/NoEngine.h"
#include "application/TestApp/Utility/CollisionMask.h"

namespace TestApp {

	/// <summary>
	/// 3Dオブジェクト用の球体コライダーコンポーネント
	/// TransformComponentと連携してワールド座標での衝突判定を行う
	/// </summary>
	struct Collider3DComponent {
		/// ワールド座標での中心位置（UpdateCollider3Dで自動更新）
		No::Vector3 worldPosition{ 0.f, 0.f, 0.f };

		/// モデル空間での半径（useScaleAsRadius = false の場合に使用）
		float radius = 0.5f;

		/// ワールド空間での実際の半径（radius * radiusMultiplier で自動計算）
		float worldRadius = 0.5f;

		/// 半径の倍率
		/// 例: 0.5f = 半分, 1.0f = 等倍, 2.0f = 2倍
		float radiusMultiplier = 1.0f;

		/// TransformComponentのscaleを半径として使用するかどうか
		/// true : worldRadius = max(scale.xyz) * radiusMultiplier
		/// false: worldRadius = radius * radiusMultiplier
		bool useScaleAsRadius = false;


		// このコライダーが「何者であるか」を示す種別
		CollisionType collisionLayer = CollisionType::None;
		// 衝突判定を行う対象の種別マスク
		CollisionType collisionMask = CollisionType::None;
		// 現在衝突しているかどうか
		bool isColliding = false;
		// 衝突相手のエンティティ
		No::Entity collidedEntity;
	};

}