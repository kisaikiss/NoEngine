#pragma once
#include "engine/Assets/Material.h"

namespace NoEngine {
namespace Component {
/// <summary>
/// マテリアルを表すコンポーネント。
/// マルチマテリアルなのでメッシュごとのマテリアルはstd::spanでこのコンポーネントに管理されている
/// </summary>
struct MaterialComponent {
	std::span<Material> materials;
	Math::Color color = Math::Color::WHITE;
	uint32_t psoId = 0;
	uint32_t rootSigId = 0;
	std::wstring psoName;
	bool drawOutline = false;
	bool enableSkinning = false;
	float shininess = 60.f;
	float enviromentCoefficient = 0.f;
};
}
}