#include "TerrainLoadSystem.h"
#include "application/ClockworksDisease/Component/Stage/StageComponent.h"

void TerrainLoadSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	for (auto e : registry.View<No::TerrainMesh, No::MeshComponent, No::TransformComponent, StageComponent>()) {
		
		auto* stage = registry.GetComponent<StageComponent>(e);
		auto* terrain = registry.GetComponent<No::TerrainMesh>(e);

		if (stage->stageColliderName != stage->preName) {
			No::LoadMeshCollider(NoEngine::AssetManager::GetFilePathFromAddressableName(stage->stageColliderName), terrain);
			stage->preName = stage->stageColliderName;
		}

		auto* transform = registry.GetComponent<No::TransformComponent>(e);
		// 親を含むワールド行列で、配置変更時だけ判定用の形状を更新する。
		No::UpdateTerrainWorld(*terrain, transform->MakeAffineMatrix4x4(registry));
	}

}

REGISTER_SYSTEM(::TerrainLoadSystem, "TerrainLoadSystem", "Gameplay")
