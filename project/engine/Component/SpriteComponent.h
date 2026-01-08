#pragma once
#include "engine/Math/Color/Color.h"
#include "engine/Math/Types/Vector2.h"
#include "engine/Math/Types/Rect.h"
#include "engine/Assets/Texture/TextureManager.h"

namespace NoEngine {
namespace Component {
struct SpriteComponent {
	TextureRef textureHandle;
	Rect uv;
	Color color{ Color::WHITE };
	Vector2 pivot{ 0.5f,0.5f };
	bool flipX = false;
	bool flipY = false;
};
}
}