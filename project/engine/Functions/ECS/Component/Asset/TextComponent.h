#pragma once
#include "engine/Assets/Font/FontManager.h"
#include "engine/Functions/ECS/Component/Asset/SpriteComponent.h"

namespace NoEngine {
namespace Component {
struct TextComponent {
	enum class HAlign { kLeft, kCenter, kRight };
	enum class VAlign { kBaseline, kTop, kMiddle };

	std::string text;
	FontRef fontHandle;

	// フォントのベイク基準ピクセルサイズ(FontManager::kBakePixelHeight)に対する倍率。
	float scale = 1.f;
	// advanceに加算する追加スペース。フォントのベイク基準ピクセル単位(scale適用前)。
	float letterSpacing = 0.f;

	int layer = 0;
	int orderInLayer = 0;

	HAlign hAlign = HAlign::kLeft;
	VAlign vAlign = VAlign::kBaseline;

	bool isVisible = true;

	SpriteSpace space = SpriteSpace::World;
	Math::Vector2 anchor = { 0.f, 0.f }; // Screen空間のみ使用。

	Math::Color color = Math::Color::WHITE;
};
}
}