#pragma once
#include "engine/NoEngine.h"
#include <string>

/// <summary>
/// フィールド配置オブジェクトの種別ID。
/// 文字列キーとの相互変換前提で、将来の種別追加を行いやすくする
/// </summary>
enum class FieldObjectType {
	Unknown = 0,
	Skydome,
	Building,
	Ground
};

/// <summary>
/// 種別ごとのデフォルト設定。
/// 配置時のモデルキーと当たり判定基準値を保持する
/// </summary>
struct FieldObjectTypeDefinition {
	FieldObjectType type = FieldObjectType::Unknown;
	std::string typeKey;
	std::string modelKey;
	bool hasCollision = false;
	bool occludesPlayerAttack = false;
	No::Vector3 baseColliderAtUnitScale = { 1.0f, 1.0f, 1.0f };
};

/// <summary>
/// 配置オブジェクト共通コンポーネント。
/// Transform編集対象を座標/大きさ/回転に限定し、種別と衝突設定を一元管理する
/// </summary>
struct FieldPlacementComponent {
	std::string objectId;
	std::string displayName;
	std::string typeKey;
	std::string modelKey;
	bool hasCollision = false;
	bool occludesPlayerAttack = false;
	No::Vector3 colliderBaseAtUnitScale = { 1.0f, 1.0f, 1.0f };
	No::Vector3 rotationEulerDeg = { 0.0f, 0.0f, 0.0f };
};
