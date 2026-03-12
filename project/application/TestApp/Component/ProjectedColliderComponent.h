#pragma once
#include "engine/NoEngine.h"
#include "application/TestApp/Utility/CollisionMask.h"

namespace TestApp {

	/// <summary>
	/// 3Dオブジェクトを2Dスクリーン座標に投影したコライダー情報
	/// Collider3DComponentの情報をProjectColliders()が毎フレーム自動更新する
	/// </summary>
	struct ProjectedColliderComponent {

		/// 投影元の3Dエンティティ
		No::Entity source3DEntity;

		/// スクリーン座標での投影位置
		No::Vector2 screenPosition{ 0.f, 0.f };

		/// スクリーン座標での半径
		/// カメラから遠いほど小さく、近いほど大きくなる
		float screenRadius = 0.f;

		/// このオブジェクトがカメラに映っているか
		/// isVisible = false の場合は衝突判定をスキップ
		bool isVisible = false;

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