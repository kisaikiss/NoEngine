#pragma once
#include "engine/Assets/Material.h"
#include "engine/Assets/AssetHandles.h"

namespace NoEngine {
enum class BlendMode {

	kNormal,

	kAdd,

	kSubtract,

	kMultiply,

	kScreen,

};

enum class RenderMode {
	kDefault,
	kToon,
	kEmissive,
};

namespace Component {
/// <summary>
/// マテリアルを表すコンポーネント。
/// マルチマテリアルなのでメッシュごとのマテリアルはstd::spanでこのコンポーネントに管理されている
/// </summary>
struct MaterialComponent {
	std::vector<Asset::MaterialHandle> handles;
	Math::Color color = Math::Color::WHITE;
	uint32_t psoId = 0;
	uint32_t rootSigId = 0;
	BlendMode blendMode = BlendMode::kNormal;
	RenderMode renderMode = RenderMode::kDefault;
	bool drawOutline = false;
	float shininess = 60.f;
	float enviromentCoefficient = 0.f;
	Math::Vector2 uvPosition = Math::Vector2::ZERO;
	float uvRotate = 0.0f;
	Math::Vector2 uvScale = Math::Vector2::UNIT_SCALE;

	// プラズマ用に追加
	float emissiveIntensity = 1.0f;
	float rimPower = 2.0f;
	float noiseScrollSpeed = 0.3f;
	std::string noiseTextureName = "noise0";
	TextureRef noiseTextureHandle;// ノイズテクスチャ
};
}
}