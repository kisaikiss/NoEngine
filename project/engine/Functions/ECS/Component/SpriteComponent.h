#pragma once
#include "engine/Math/MathInclude.h"
#include "engine/Assets/Texture/TextureManager.h"

namespace NoEngine {
namespace Component {
struct SpriteComponent {
	Math::Vector2 pivot{ 0.5f,0.5f };
	bool flipX = false;
	bool flipY = false;
	Rect uv{};
	uint32_t layer = 0;
	uint32_t orderInLayer = 0;
	TextureRef textureHandle;
	Math::Color color = { Math::Color::WHITE};
	std::string name;
	bool isVisible = true;
	float fill = 0.0f;				// 0..1 （UV.x に対する切り詰め量）
	int useMask = 0;				// マスクを使用するかどうか（UI 用）
	TextureRef maskTextureHandle;	// マスクテクスチャ（UI 用）
};
}
}