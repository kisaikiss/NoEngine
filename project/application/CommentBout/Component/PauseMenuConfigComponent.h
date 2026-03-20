#pragma once
#include "engine/NoEngine.h"

struct PauseMenuConfigComponent {
	No::Vector2 titleStartPosition{ 640.0f, 120.0f };
	No::Vector2 titleEndPosition{ 640.0f, 220.0f };
	No::Vector2 titleSize{ 360.0f, 90.0f };

	No::Vector2 itemBaseStartPosition{ 640.0f, 520.0f };
	No::Vector2 itemBaseEndPosition{ 640.0f, 360.0f };
	No::Vector2 itemSize{ 340.0f, 70.0f };
	float itemSpacing = 80.0f;
	No::Vector2 cursorStartOffset{ -300.0f, 0.0f };
	No::Vector2 cursorEndOffset{ -240.0f, 0.0f };
	No::Vector2 cursorSize{ 42.0f, 42.0f };

	No::Vector2 menuBgStartPosition{ 640.0f, 760.0f };
	No::Vector2 menuBgEndPosition{ 640.0f, 430.0f };
	No::Vector2 menuBgStartSize{ 980.0f, 540.0f };
	No::Vector2 menuBgEndSize{ 980.0f, 540.0f };
	No::Color menuBgColor{ 0.08f, 0.08f, 0.12f, 0.70f };

	No::Vector2 panelLineStartPosition{ 640.0f, -40.0f };
	No::Vector2 panelLineEndPosition{ 640.0f, 330.0f };
	No::Vector2 panelLineStartSize{ 980.0f, 8.0f };
	No::Vector2 panelLineEndSize{ 980.0f, 8.0f };
	No::Color panelLineColor{ 1.0f, 1.0f, 1.0f, 0.95f };

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
	int menuBgLayer = 905;
	int panelLineLayer = 907;
	int titleLayer = 910;
	int itemLayer = 920;
	int cursorLayer = 930;
};