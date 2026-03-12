#pragma once
#include "engine/NoEngine.h"
#include "application/TestApp/Utility/CollisionMask.h"

namespace TestApp {

	/// <summary>
	/// 3Dコライダーの形状種別
	/// </summary>
	enum class ShapeType3D {
		Sphere,	// 球体（デフォルト）
		Box,	// AABB（軸平行境界ボックス）
	};

	/// <summary>
	/// 3Dオブジェクト用のコライダーコンポーネント
	/// </summary>
	struct Collider3DComponent {
		/// このコライダーの形状種別
		ShapeType3D shapeType = ShapeType3D::Sphere;

		/// ワールド座標での中心位置（UpdateCollider3Dで自動更新）
		No::Vector3 worldPosition{ 0.f, 0.f, 0.f };


		// Sphere 用パラメータ
		/// モデル空間での半径（useScaleAsRadius = false の場合に使用）
		float radius = 0.5f;

		/// ワールド空間での実際の半径（自動計算）
		float worldRadius = 0.5f;

		/// 半径の倍率
		/// 例: 0.5f = 半分, 1.0f = 等倍, 2.0f = 2倍
		float radiusMultiplier = 1.0f;

		/// TransformComponentのscaleを半径として使用するかどうか
		/// true : worldRadius = max(scale.xyz) * radiusMultiplier
		/// false: worldRadius = radius * radiusMultiplier
		bool useScaleAsRadius = false;


		// Box 用パラメータ
		/// モデル空間でのボックスのフルサイズ（幅・高さ・奥行き）
		/// useScaleAsBox = false の場合に使用
		No::Vector3 boxSize{ 1.f, 1.f, 1.f };

		/// ワールド空間での実際のボックスサイズ（自動計算）
		No::Vector3 worldBoxSize{ 1.f, 1.f, 1.f };

		/// TransformComponentのscaleをボックスサイズとして使用するかどうか
		/// true : worldBoxSize = scale * boxSizeMultiplier
		/// false: worldBoxSize = boxSize * boxSizeMultiplier
		bool useScaleAsBox = true;

		/// ボックスサイズの倍率
		No::Vector3 boxSizeMultiplier{ 1.f, 1.f, 1.f };


		// ========================================
		// 共通パラメータ
		// ========================================

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