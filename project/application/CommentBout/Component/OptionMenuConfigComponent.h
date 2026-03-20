#pragma once
#include "engine/NoEngine.h"

struct OptionMenuConfigComponent {
	No::Vector2 dimStartPosition{ 198.700f, 430.000f };
	No::Vector2 dimEndPosition{ 198.700f, 430.000f };
	No::Vector2 dimSize{ 621.300f, 952.100f };
	No::Color dimColor{ 0.000f, 0.000f, 0.000f, 0.950f };
	float dimRotation = -9.700f;

	No::Vector2 bgStartPosition{ 640.000f, 760.000f };
	No::Vector2 bgEndPosition{ 1063.100f, 491.500f };
	No::Vector2 bgStartSize{ 960.000f, 520.000f };
	No::Vector2 bgEndSize{ 979.300f, 652.400f };
	No::Color bgColor{ 1.000f, 0.600f, 0.000f, 0.900f };
	float bgRotation = -0.280f;

	No::Vector2 lineStartPosition{ 857.200f, -130.200f };
	No::Vector2 lineEndPosition{ 943.000f, 179.800f };
	No::Vector2 lineSize{ 919.700f, 16.000f };
	No::Color lineColor{ 1.000f, 1.000f, 1.000f, 0.950f };
	float lineRotation = -0.280f;

	No::Vector2 titleStartPosition{ 860.0f, -60.0f };
	No::Vector2 titleEndPosition{ 920.0f, 120.0f };
	No::Vector2 titleSize{ 280.0f, 58.0f };

	No::Vector2 itemBaseStartPosition{ 640.000f, 860.000f };
	No::Vector2 itemBaseEndPosition{ 935.400f, 333.300f };
	No::Vector2 itemSize{ 620.000f, 52.700f };
	float itemSpacing = 69.600f;
	No::Color itemColor{ 0.200f, 0.200f, 0.240f, 0.900f };
	No::Color selectedItemColor{ 0.420f, 0.340f, 0.120f, 0.960f };

	No::Vector2 labelOffset{ -180.0f, 0.0f };
	No::Vector2 labelSize{ 250.0f, 42.0f };
	No::Vector2 backLabelStartPosition{ 700.0f, 860.0f };
	No::Vector2 backLabelEndPosition{ 760.0f, 612.0f };
	No::Vector2 backLabelSize{ 220.0f, 44.0f };

	No::Vector2 cursorSelectOffset{ -260.0f, 0.0f };
	No::Vector2 cursorEditOffset{ -40.0f, 0.0f };
	No::Vector2 cursorSize{ 14.0f, 48.0f };
	No::Color cursorColor{ 1.0f, 1.0f, 1.0f, 0.95f };
	No::Color cursorEditColor{ 1.0f, 0.85f, 0.25f, 1.0f };

	No::Vector2 barOffset{ 110.000f, 0.000f };
	No::Vector2 barBaseSize{ 360.000f, 16.000f };
	No::Vector2 barFillMinSize{ 8.000f, 16.000f };
	No::Color barBaseColor{ 0.100f, 0.100f, 0.100f, 0.950f };
	No::Color barFillColor{ 0.950f, 0.900f, 0.280f, 0.950f };

	No::Vector2 toggleOffset{ 9.000f, 0.000f };
	No::Vector2 toggleSize{ 154.700f, 34.800f };
	No::Color toggleOnColor{ 0.200f, 0.750f, 0.300f, 0.950f };
	No::Color toggleOffColor{ 0.700f, 0.200f, 0.200f, 0.950f };

	float openDuration = 0.220f;
	float closeDuration = 0.180f;
	float volumeStep = 0.050f;

	int dimLayer = 940;
	int bgLayer = 945;
	int lineLayer = 958;
	int itemLayer = 950;
	int labelLayer = 951;
	int cursorLayer = 952;
	int barBaseLayer = 955;
	int barFillLayer = 956;
	int toggleLayer = 957;
};