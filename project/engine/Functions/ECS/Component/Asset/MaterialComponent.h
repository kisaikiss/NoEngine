#pragma once
#include "engine/Assets/Material.h"
#include "engine/Assets/AssetHandles.h"

namespace NoEngine {
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
	std::wstring psoName;
	bool drawOutline = false;
	float shininess = 60.f;
	float enviromentCoefficient = 0.f;
	Math::Vector2 uvPosition = Math::Vector2::ZERO;
	float uvRotate = 0.0f;
	Math::Vector2 uvScale = Math::Vector2::UNIT_SCALE;
};
}
}