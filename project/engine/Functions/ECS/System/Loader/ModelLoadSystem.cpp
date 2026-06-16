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
	for (auto e : view) {
		auto* mesh = registry.GetComponent<MeshComponent>(e);
		auto asset = ModelSaver::Get().LoadOrGetModel(AssetManager::GetFilePathFromAddressableName(mesh->meshName));
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