#pragma once
#include "engine/Assets/Font/FontManager.h"

namespace NoEngine {
namespace Component {
struct TextComponent {
	enum class HAlign { kLeft, kCenter, kRight };
	enum class VAlign { kBaseline, kTop, kMiddle };

	// note: 毎フレーム書き換わるHUD文字列を想定しているためstd::stringのまま。
	// ECSのホットパス(物理/衝突などQuery頻度が高い箇所)には使わない前提で許容している。
	std::string text;
	FontRef fontHandle;

	float scale = 1.f;
	float letterSpacing = 0.f; // advanceに加算するピクセル単位の追加スペース

	int layer = 0;
	int orderInLayer = 0;

	HAlign hAlign = HAlign::kLeft;
	VAlign vAlign = VAlign::kBaseline;

	bool isVisible = true;
};
}
}