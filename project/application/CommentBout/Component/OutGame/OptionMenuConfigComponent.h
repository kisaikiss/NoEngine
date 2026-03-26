#pragma once
#include "engine/NoEngine.h"

struct OptionMenuConfigComponent {
	No::Vector2 dimStartPosition{ 198.70f, 430.00f };
	No::Vector2 dimEndPosition{ 198.70f, 430.00f };
	No::Vector2 dimSize{ 621.30f, 952.10f };
	No::Color dimColor{ 0.00f, 0.00f, 0.00f, 0.95f };
	float dimRotation = -9.70f;

	No::Vector2 bgStartPosition{ 640.00f, 760.00f };
	No::Vector2 bgEndPosition{ 1063.10f, 491.50f };
	No::Vector2 bgStartSize{ 960.00f, 520.00f };
	No::Vector2 bgEndSize{ 979.30f, 652.40f };
	No::Color bgColor{ 1.00f, 0.60f, 0.00f, 0.90f };
	float bgRotation = -0.28f;

	No::Vector2 lineStartPosition{ 857.20f, -130.20f };
	No::Vector2 lineEndPosition{ 943.00f, 179.80f };
	No::Vector2 lineSize{ 919.70f, 16.00f };
	No::Color lineColor{ 1.00f, 1.00f, 1.00f, 0.95f };
	float lineRotation = -0.28f;

	No::Vector2 titleStartPosition{ 1383.20f, 248.40f };
	No::Vector2 titleEndPosition{ 1132.20f, 230.60f };
	No::Vector2 titleSize{ 301.80f, 65.50f };
	float titleRotation = 0.10f;

	No::Vector2 itemBaseStartPosition{ 640.00f, 860.00f };
	No::Vector2 itemBaseEndPosition{ 935.40f, 333.30f };
	No::Vector2 itemSize{ 620.00f, 52.70f };
	No::Vector2 backItemStartPosition{ 640.00f, 1138.40f };
	No::Vector2 backItemEndPosition{ 1146.70f, 611.70f };
	No::Vector2 backItemSize{ 195.90f, 52.70f };
	float itemSpacing = 69.60f;
	No::Color itemColor{ 0.20f, 0.20f, 0.24f, 0.90f };
	No::Color selectedItemColor{ 0.42f, 0.34f, 0.12f, 0.96f };

	No::Vector2 labelOffset{ -191.50f, 0.00f };
	No::Vector2 labelSize{ 226.50f, 42.00f };
	No::Vector2 backLabelStartPosition{ 760.00f, 860.00f };
	No::Vector2 backLabelEndPosition{ 1168.40f, 611.70f };
	No::Vector2 backLabelSize{ 103.20f, 50.20f };

	No::Vector2 cursorSelectOffset{ -276.20f, 0.00f };
	No::Vector2 cursorBackOffset{ 146.60f, 0.00f };
	No::Vector2 cursorSize{ 28.00f, 28.00f };
	No::Color cursorColor{ 1.00f, 1.00f, 1.00f, 0.95f };
	No::Color cursorEditColor{ 1.00f, 0.85f, 0.25f, 1.00f };

	No::Vector2 barOffset{ 110.00f, 0.00f };
	No::Vector2 barBaseSize{ 360.00f, 16.00f };
	No::Vector2 barFillMinSize{ 8.00f, 16.00f };
	No::Color barBaseColor{ 0.10f, 0.10f, 0.10f, 0.95f };
	No::Color barFillColor{ 0.95f, 0.90f, 0.28f, 0.95f };

	No::Vector2 toggleOffset{ 98.30f, 0.00f };
	No::Vector2 toggleSize{ 126.20f, 52.00f };
	No::Color toggleOnColor{ 0.20f, 0.75f, 0.30f, 0.95f };
	No::Color toggleOffColor{ 0.70f, 0.20f, 0.20f, 0.95f };

	// 編集中に非選択行へ重ねる暗幕
	No::Color itemOverlayColor{ 0.00f, 0.00f, 0.00f, 0.65f };

	float openDuration = 0.22f;
	float closeDuration = 0.18f;
	float volumeStep = 0.05f;
	float confirmDuration = 0.12f;
	float confirmScale = 1.08f;

	int dimLayer = 940;
	int bgLayer = 945;
	int lineLayer = 958;
	int itemLayer = 950;
	int labelLayer = 951;
	int cursorLayer = 952;
	int barBaseLayer = 955;
	int barFillLayer = 956;
	int toggleLayer = 957;
	int itemOverlayLayer = 959;
};