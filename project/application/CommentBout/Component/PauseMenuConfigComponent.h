#pragma once
#include "engine/NoEngine.h"

struct PauseMenuConfigComponent {
	No::Vector2 titlePosition{ 640.0f, 220.0f };
	No::Vector2 titleSize{ 360.0f, 90.0f };

	No::Vector2 itemBasePosition{ 640.0f, 360.0f };
	No::Vector2 itemSize{ 340.0f, 70.0f };
	float itemSpacing = 80.0f;
	No::Vector2 cursorOffset{ -240.0f, 0.0f };
	No::Vector2 cursorSize{ 42.0f, 42.0f };

	float dimAlpha = 0.65f;
	float openDuration = 0.2f;
	float closeDuration = 0.2f;
	float optionOpenDuration = 0.2f;
	float optionCloseDuration = 0.2f;
	float confirmDuration = 0.12f;
	float selectedScale = 1.08f;
	float confirmScale = 1.18f;
	int easeType = 0;

	int dimLayer = 900;
	int titleLayer = 910;
	int itemLayer = 920;
	int cursorLayer = 930;
};
