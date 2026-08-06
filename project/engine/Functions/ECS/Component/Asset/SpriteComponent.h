#pragma once
#include "engine/Math/MathInclude.h"
#include "engine/Assets/Texture/TextureManager.h"

namespace NoEngine {
enum class SpriteSpace {
	World,   // カメラの影響を受ける
	Screen,  // 常にウィンドウ基準（UI用）
};

namespace Component {
struct SpriteComponent {
	SpriteSpace space = SpriteSpace::World;
	Math::Vector2 anchor = { 0.f, 0.f }; // 0,0=左上 1,1=右下 0.5,0.5=中央 など。 0-1正規化。UIのみ使用
	Math::Vector2 pivot{ 0.5f,0.5f };
	bool flipX = false;
	bool flipY = false;
	Rect uv{};
	uint32_t layer = 0;
	uint32_t orderInLayer = 0;
	TextureRef textureHandle;
	std::string textureName;
	Math::Color color = { Math::Color::WHITE};
	std::string name;
	bool isVisible = true;
	float nonRenderAngle = 0.0f;	// 描画しない角度（円形ゲージなどに使う）
	float fill = 0.0f;				// 0..1 （UV.x に対する切り詰め量）
	bool useMask = false;				// マスクを使用するかどうか（UI 用）
	std::string maskTextureName;
	TextureRef maskTextureHandle;	// マスクテクスチャ（UI 用）
};
}
}