#include "DeathEffectComponent.h"
#include "engine/Functions/ECS/Registry.h"
#include "engine/Functions/ECS/Component/TransformComponent.h"
#include "engine/Functions/ECS/Component/MaterialComponent.h"
#include "engine/Functions/ECS/Component/MeshComponent.h"
#include "engine/Assets/ModelLoader.h"
#include "engine/Functions/Renderer/RenderSystem.h"
#include <cmath>

namespace {
	static const float kPI = 3.14159265359f;
}

namespace DeathEffectHelper {

void SpawnDeathEffect(NoEngine::ECS::Registry& registry, const NoEngine::Math::Vector3& centerPos, const DeathEffectConfig& config) {
	float angleStep = (2.0f * kPI) / config.particleCount;

	// 各パーティクルエンティティを生成
	for (int i = 0; i < config.particleCount; ++i) {
		NoEngine::ECS::Entity particleEntity = registry.GenerateEntity();

		// DeathParticleComponent
		auto* particle = registry.AddComponent<DeathParticleComponent>(particleEntity);
		particle->startPosition = centerPos;
		particle->timer = 0.0f;
		particle->duration = config.duration;
		particle->startColor = config.color;
		particle->startScale = config.particleScale;

		float angle = angleStep * i;
		particle->targetPosition = centerPos;
		particle->targetPosition.x += config.spreadDistance * std::cos(angle);
		particle->targetPosition.y += config.spreadDistance * std::sin(angle);

		// TransformComponent
		auto* transform = registry.AddComponent<NoEngine::Component::TransformComponent>(particleEntity);
		transform->translate = centerPos;
		transform->scale = NoEngine::Math::Vector3(config.particleScale, config.particleScale, config.particleScale);

		// MeshComponent
		auto* mesh = registry.AddComponent<NoEngine::Component::MeshComponent>(particleEntity);
		NoEngine::Asset::ModelLoader::GetModel("ParticleModel", mesh);

		// MaterialComponent
		auto* material = registry.AddComponent<NoEngine::Component::MaterialComponent>(particleEntity);
		material->materials = NoEngine::Asset::ModelLoader::GetMaterial("ParticleModel");
		material->color = config.color;
		material->psoName = L"Renderer : Default PSO";
		material->psoId = NoEngine::Render::GetPSOID(material->psoName);
		material->rootSigId = NoEngine::Render::GetRootSignatureID(material->psoName);
	}
}

}
