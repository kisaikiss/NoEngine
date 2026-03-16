#pragma once
#include "engine/NoEngine.h"
#include "application/TestApp/Utility/CollisionMask.h"
#include <vector>

namespace TestApp {

	/// <summary>
	/// 3Dオブジェクトを2Dスクリーン座標に投影したコライダー情報
	/// </summary>
	struct ProjectedColliderComponent {

		/// 投影元の3Dエンティティ
		No::Entity source3DEntity;

		/// スクリーン座標での投影中心位置（Sphere / Box 共通）
		/// Box の場合は screenMin/Max の中心として自動計算される
		No::Vector2 screenPosition{ 0.f, 0.f };


		// Sphere 用
		/// スクリーン座標での半径（Sphere のみ使用）
		/// カメラから遠いほど小さく、近いほど大きくなる
		float screenRadius = 0.f;


		// Box 用（8頂点投影結果）
		/// debug用Boxの8頂点を投影した結果のスクリーン AABB min左上 max右下
		No::Vector2 screenMin{ 0.f, 0.f };
		No::Vector2 screenMax{ 0.f, 0.f };

		/// Box の有効な投影頂点から計算した凸包（衝突判定用）
		/// 斜め視点でも外接 AABB より正確な衝突判定が可能
		/// ProjectColliders で毎フレーム更新される
		std::vector<No::Vector2> convexHull;


		// 共通
		/// true = Box として投影された（CheckProjectedVs2D で判定方式を切り替え）
		/// false = Sphere として投影された
		bool isBox = false;

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