#pragma once
#include "engine/NoEngine.h"
#include "application/TestApp/Utility/CollisionMask.h"

namespace TestApp {

	/// <summary>
	/// 2Dスプライト用のAABBコライダーコンポーネント
	/// Transform2DComponentと連携してスクリーン座標での衝突判定を行う
	/// </summary>
	struct Collider2DComponent {

		/// スクリーン座標での中心位置（UpdateCollider2Dで自動更新）
		No::Vector2 screenPosition{ 0.f, 0.f };

		/// スクリーン空間での実際のサイズ（自動計算）
		No::Vector2 worldSize{ 100.f, 100.f };

		/// AABBのフルサイズ（幅と高さ）
		/// useTransformAsSize = false の場合に直接使用
		No::Vector2 size{ 100.f, 100.f };

		/// サイズの倍率
		/// 例: (0.8f, 0.8f) = 見た目の80%のコライダー, (1.0f, 1.0f) = 等倍
		No::Vector2 sizeMultiplier{ 1.0f, 1.0f };

		/// Transform2DComponentのscaleをサイズとして使用するかどうか
		/// true : worldSize = Transform2D.scale * sizeMultiplier
		/// false: worldSize = size * sizeMultiplier
		bool useTransformAsSize = true;

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