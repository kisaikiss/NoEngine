#pragma once
#include "engine/NoEngine.h"

struct OptionMenuConfigComponent {
	No::Vector2 dimStartPosition{ 640.0f, 360.0f };
	No::Vector2 dimEndPosition{ 640.0f, 360.0f };
	No::Vector2 dimSize{ 1280.0f, 720.0f };
	No::Color dimColor{ 0.0f, 0.0f, 0.0f, 0.60f };

	No::Vector2 bgStartPosition{ 640.0f, 760.0f };
	No::Vector2 bgEndPosition{ 640.0f, 400.0f };
	No::Vector2 bgStartSize{ 960.0f, 520.0f };
	No::Vector2 bgEndSize{ 960.0f, 520.0f };
	No::Color bgColor{ 0.06f, 0.06f, 0.08f, 0.90f };

	No::Vector2 lineStartPosition{ 640.0f, -20.0f };
	No::Vector2 lineEndPosition{ 640.0f, 220.0f };
	No::Vector2 lineSize{ 920.0f, 6.0f };
	No::Color lineColor{ 1.0f, 1.0f, 1.0f, 0.95f };

	No::Vector2 itemBaseStartPosition{ 640.0f, 860.0f };
	No::Vector2 itemBaseEndPosition{ 640.0f, 300.0f };
	No::Vector2 itemSize{ 760.0f, 60.0f };
	float itemSpacing = 72.0f;
	No::Color itemColor{ 0.20f, 0.20f, 0.24f, 0.90f };
	No::Color selectedItemColor{ 0.42f, 0.34f, 0.12f, 0.96f };

	No::Vector2 barOffset{ 160.0f, 0.0f };
	No::Vector2 barBaseSize{ 360.0f, 16.0f };
	No::Vector2 barFillMinSize{ 8.0f, 16.0f };
	No::Color barBaseColor{ 0.10f, 0.10f, 0.10f, 0.95f };
	No::Color barFillColor{ 0.95f, 0.90f, 0.28f, 0.95f };

	No::Vector2 toggleOffset{ 170.0f, 0.0f };
	No::Vector2 toggleSize{ 120.0f, 28.0f };
	No::Color toggleOnColor{ 0.20f, 0.75f, 0.30f, 0.95f };
	No::Color toggleOffColor{ 0.70f, 0.20f, 0.20f, 0.95f };

	float openDuration = 0.22f;
	float closeDuration = 0.18f;
	float volumeStep = 0.05f;

	int dimLayer = 940;
	int bgLayer = 945;
	int lineLayer = 946;
	int itemLayer = 950;
	int barBaseLayer = 955;
	int barFillLayer = 956;
	int toggleLayer = 957;
};
