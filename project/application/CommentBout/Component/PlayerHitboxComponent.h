#pragma once
#include "engine/NoEngine.h"

struct PlayerHitboxComponent {
	No::Entity playerEntity = No::nullEntity;
	No::Vector3 worldOffset = { 0.0f, 0.0f, 0.0f };
	No::Vector3 worldSize = { 0.8f, 1.2f, 0.8f };
	No::Vector3 sizeMultiplier = { 1.0f, 1.0f, 1.0f };
	float spritePlaneZ = 0.8f;
	bool fitToSprite = true;
	bool drawDebug = true;
};
