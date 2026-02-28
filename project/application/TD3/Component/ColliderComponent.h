#pragma once
#include "engine/Math/MathInclude.h"
#include "engine/Functions/ECS/Entity.h"

/// <summary>
/// TD3用のコライダーマスク定義
/// </summary>
enum ColliderMask
{
	kPlayer = 1 << 0,		// プレイヤー
	kEnemy = 1 << 1,		// 敵
	kPlayerBullet = 1 << 2, // プレイヤーの弾
	kShockwave = 1 << 3     // 衝撃波
};

/// <summary>
/// 球体コライダーコンポーネント
/// TransformComponentのtranslateとscaleから自動的にワールド座標と半径を計算する
/// </summary>
struct SphereColliderComponent
{
	No::Vector3 center;											// ワールド座標での中心位置（UpdateColliderで自動更新）
	float radius = 0.5f;										// モデル空間での半径（Scale適用前の値）
	float worldRadius = 0.5f;									// ワールド空間での半径（radius * maxScale）
	bool isCollied = false;										// 衝突フラグ（毎フレームリセット）
	uint32_t colliderType = 0xffffffff;							// このコライダーの種類（ColliderMask）
	uint32_t collideMask = 0xffffffff;							// 衝突判定する対象の種類（ColliderMask）
	ColliderMask colliedWith = static_cast<ColliderMask>(0);	// 衝突相手の種類
	NoEngine::ECS::Entity colliedEntity;						// 衝突相手のエンティティ
};

/// <summary>
/// ボックスコライダーコンポーネント(仮置き)
/// </summary>
struct BoxColliderComponent
{
	No::Vector3 center;
	No::Vector3 size = { 1.0f, 1.0f, 1.0f };
	No::Vector3 worldSize = { 1.0f, 1.0f, 1.0f };
	bool isCollied = false;
	uint32_t colliderType = 0xffffffff;
	uint32_t collideMask = 0xffffffff;
};