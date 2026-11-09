#pragma once
#include "engine/Assets/Texture/TextureManager.h"
#include "engine/Math/Color/Color.h"
#include "engine/Math/Types/Vector2.h"

namespace NoEngine {
enum class BlendMode {

	kNormal,

	kAdd,

	kSubtract,

	kMultiply,

	kScreen,

	kAlpha,

};

enum class RenderMode {
	kDefault,
	kToon,
	kEmissive,
};

namespace Component {
/// <summary>
/// エンティティの全サブメッシュに共通のマテリアル設定。
/// </summary>
struct MaterialComponent {
	// テクスチャのAddressableNameを指定すると、全サブメッシュのテクスチャを上書きする。
	std::string textureName;
	TextureRef textureHandle;
	uint64_t loadedTextureGeneration = 0;
	std::string loadedTextureName;
	Math::Color color = Math::Color::WHITE;
	uint32_t psoId = 0;
	uint32_t rootSigId = 0;
	BlendMode blendMode = BlendMode::kNormal;
	RenderMode renderMode = RenderMode::kDefault;
	bool drawOutline = false;
	bool unlit = false;          // true : ライトの影響を受けない
	bool receiveShadow = true;   // false : 他オブジェクトが落とす影の影響を受けない
	bool castShadow = true;
	float shininess = 60.f;
	float environmentCoefficient = 0.f;
	Math::Vector2 uvPosition = Math::Vector2::ZERO;
	float uvRotate = 0.0f;
	Math::Vector2 uvScale = Math::Vector2::UNIT_SCALE;

	// プラズマ用に追加
	float emissiveIntensity = 1.0f;
	float rimPower = 2.0f;
	float noiseScrollSpeed = 0.3f;
	Math::Color edgeColor = Math::Color::WHITE;
	std::string noiseTextureName = "noise0";
	std::string loadedNoiseTextureName;
	uint64_t loadedNoiseTextureGeneration = 0;
	TextureRef noiseTextureHandle;// ノイズテクスチャ
};
}
}
