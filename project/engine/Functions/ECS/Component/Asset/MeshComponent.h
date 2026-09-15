#pragma once
#include "engine/Assets/AssetHandles.h"

namespace NoEngine {
namespace Component {
struct MeshComponent {
	Asset::MeshHandle handle;
	std::string meshName;
	bool isVisible = true;
	uint64_t loadedGeneration = 0;
	std::string loadedMeshName;
};
}
}