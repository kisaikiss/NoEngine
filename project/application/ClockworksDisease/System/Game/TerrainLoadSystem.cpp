#include "TerrainLoadSystem.h"
#include "application/ClockworksDisease/Component/Stage/StageComponent.h"

void TerrainLoadSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	for (auto e : registry.View<No::TerrainMesh, No::MeshComponent, No::TransformComponent, StageComponent>()) {
		
		auto* stage = registry.GetComponent<StageComponent>(e);

		if (stage->stageColliderName != stage->preName) {
			auto* terrain = registry.GetComponent<No::TerrainMesh>(e);
			No::LoadMeshCollider(NoEngine::AssetManager::GetFilePathFromAddressableName(stage->stageColliderName), terrain);
			stage->preName = stage->stageColliderName;
		}

		auto* transform = registry.GetComponent<No::TransformComponent>(e);
		transform->translate = No::Vector3::ZERO;
		transform->rotation = No::Quaternion::IDENTITY;
		transform->scale = No::Vector3::UNIT_SCALE;
	}

}
