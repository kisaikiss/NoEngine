#pragma once
#include "application/CommentBout/Component/RailCameraComponent.h"
#include <map>

struct EnemyTypePreset {
	float modelScale = 0.7f;
	float shootInterval = 1.0f;
	float bulletSpeed = 10.0f;
	int bulletDamage = 1;
	float bulletLifetime = 4.0f;
	float targetDepthFromCamera = 1.0f;
	int minHp = 1;
};

using EnemyTypePresetMap = std::map<RailEnemyType, EnemyTypePreset>;

std::string MakeEnemyTypePresetFilePath();
bool LoadEnemyTypePresetMap(EnemyTypePresetMap& outPresets);
EnemyTypePreset GetEnemyTypePresetOrDefault(const EnemyTypePresetMap& presets, RailEnemyType type);
