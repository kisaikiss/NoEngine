#include "stdafx.h"
#include "ModelLoadSystem.h"
#include "engine/Functions/ECS/Component/Asset/MeshComponent.h"
#include "engine/Functions/ECS/Component/Asset/MaterialComponent.h"
#include "engine/Functions/ECS/Component/Asset/AnimatorComponent.h"

#include "engine/Assets/Model/ModelSaver.h"
#include "engine/Assets/AssetManager.h"

namespace NoEngine {
namespace ECS {
using namespace Component;
void ModelLoadSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto view = registry.View<MeshComponent>();
	const uint64_t currentGeneration = AssetManager::GetAddressableGeneration();

	for (auto e : view) {
		auto* mesh = registry.GetComponent<MeshComponent>(e);
		const bool tableChanged = (mesh->loadedGeneration != currentGeneration);
		const bool nameChanged = (mesh->loadedMeshName != mesh->meshName);

		if (!tableChanged && !nameChanged) {
			continue;
		}

		mesh->loadedGeneration = currentGeneration;
		mesh->loadedMeshName = mesh->meshName;

		std::string path = AssetManager::GetFilePathFromAddressableName(mesh->meshName);
		if (path.empty()) {
			continue; // AddressableName解決失敗。AssetManager側で警告済み
		}

		auto asset = ModelSaver::Get().LoadOrGetModel(path);
		mesh->handle = asset.mesh;
		if (registry.Has<MaterialComponent>(e)) {
			auto* material = registry.GetComponent<MaterialComponent>(e);
			material->handles = asset.materials;
		}
		if (registry.Has<AnimatorComponent>(e)) {
			auto* animator = registry.GetComponent<AnimatorComponent>(e);
			animator->animationHandles = asset.animations;
			animator->skeletonHandle = asset.skeleton;
		}
	}
}
}
}