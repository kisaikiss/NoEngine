#pragma once
#include "engine/Math/MathInclude.h"
#include "engine/Assets/Texture/TextureManager.h"

namespace NoEngine {
namespace Component {
struct SpriteComponent {
	Vector2 pivot{ 0.5f,0.5f };
	bool flipX = false;
	bool flipY = false;
	Rect uv{};
	uint32_t layer = 0;
	uint32_t orderInLayer = 0;
	TextureRef textureHandle;
	Color color = {Color::WHITE};
	std::string name;
};
}
}