#pragma once
#include "engine/NoEngine.h"

struct TitleMenuConfigComponent {
	No::Vector2 backgroundPosition{ 640.0f, 360.0f };
	No::Vector2 backgroundSize{ 1280.0f, 720.0f };
	float backgroundRotation = 0.0f;
	No::Color backgroundColor{ 0.08f, 0.08f, 0.12f, 1.0f };

	No::Vector2 panelPosition{ 640.0f, 360.0f };
	No::Vector2 panelSize{ 1080.0f, 640.0f };
	float panelRotation = -0.08f;
	No::Color panelColor{ 0.95f, 0.55f, 0.12f, 0.92f };

	No::Vector2 logoPosition{ 640.0f, 180.0f };
	No::Vector2 logoSize{ 620.0f, 180.0f };
	float logoBaseRotation = 0.0f;

	No::Vector2 itemBasePosition{ 640.0f, 395.0f };
	No::Vector2 itemSize{ 420.0f, 90.0f };
	float itemSpacing = 95.0f;

	No::Vector2 cursorOffset{ -300.0f, 0.0f };
	No::Vector2 cursorSize{ 28.0f, 60.0f };

	No::Color itemColor{ 0.85f, 0.85f, 0.85f, 1.0f };
	No::Color selectedItemColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	No::Color cursorColor{ 1.0f, 0.95f, 0.35f, 1.0f };

	float selectedScale = 1.08f;
	float confirmScale = 1.18f;
	float confirmDuration = 0.12f;

	float logoMotionPosAmplitude = 10.0f;
	float logoMotionPosSpeed = 1.8f;
	float logoMotionScaleAmplitude = 0.025f;
	float logoMotionScaleSpeed = 1.5f;
	float logoMotionRotateAmplitude = 0.02f;
	float logoMotionRotateSpeed = 1.2f;

	int backgroundLayer = 880;
	int panelLayer = 885;
	int logoLayer = 890;
	int itemLayer = 900;
	int cursorLayer = 910;
};
