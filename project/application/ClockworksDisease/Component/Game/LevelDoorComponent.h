#pragma once
#include "engine/NoEngine.h"

struct LevelDoorComponent {
	uint32_t requiredLevel = 2;
	float activationDistance = 3.0f;
	float animationDuration = 0.8f;
	No::Vector3 digitsOffset = No::Vector3(0.0f, 0.2f, 5.1f);

	// 実行時のみの状態。これらの値は、シーンが読み込まれる際に再作成されます。
	float animationTimer = 0.0f;
	No::Vector3 openingStartPosition = No::Vector3::ZERO;
	bool isOpening = false;
	std::vector<No::Entity> levelDigits;
};
