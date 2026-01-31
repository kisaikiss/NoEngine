#pragma once
#include "engine/Assets/Material.h"

namespace NoEngine {
namespace Component {
struct MaterialComponent {
	std::span<Material> materials;
	Color color = Color::WHITE;
	uint32_t psoId = 0;
	uint32_t rootSigId = 0;
	std::wstring psoName;
	bool drawOutline = false;
	bool enableSkinning = false;
};
}
}