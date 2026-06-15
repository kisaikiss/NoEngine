#pragma once
#include "engine/Assets/Mesh.h"
#include "engine/Assets/AssetHandles.h"

namespace NoEngine {
namespace Component {
struct MeshComponent {
	Mesh* mesh;
	Asset::MeshHandle handle;
	std::string meshName;
	bool isVisible = true;
};
}
}