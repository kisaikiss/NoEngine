#pragma once
#include "application/CommentBout/Component/RailCameraComponent.h"
#include <map>

/// <summary>
/// 敵種類ごとのプリセット設定構造体
/// </summary>
struct EnemyTypePreset {
	float modelScale = 0.7f;
	No::Vector3 baseColliderBox = { 1.0f, 1.0f, 1.0f };
	float shootInterval = 1.0f;
	float bulletSpeed = 10.0f;
	int bulletDamage = 1;
	float bulletLifetime = 4.0f;
	float targetDepthFromCamera = 1.0f;
	float shootDistanceMax = 25.0f;
	float despawnBehindDistance = 12.0f;
	int minHp = 1;
};

using EnemyTypePresetMap = std::map<RailEnemyType, EnemyTypePreset>;

std::string MakeEnemyTypePresetFilePath();
bool LoadEnemyTypePresetMap(EnemyTypePresetMap& outPresets);
bool SaveEnemyTypePresetMap(const EnemyTypePresetMap& presets);
EnemyTypePreset GetEnemyTypePresetOrDefault(const EnemyTypePresetMap& presets, RailEnemyType type);
