#pragma once
#include "engine/Assets/AssetHandles.h"

namespace NoEngine {
namespace Component {
struct MeshComponent {
	Asset::MeshHandle handle;
	std::string meshName;
	bool isVisible = true;
};
}
}