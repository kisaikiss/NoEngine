#include "DeathEffectSystem.h"
#include "../Component/DeathEffectComponent.h"
#include "engine/Functions/ECS/Component/TransformComponent.h"
#include "engine/Functions/ECS/Component/MaterialComponent.h"
#include "engine/Functions/ECS/Component/MeshComponent.h"
#include "engine/Assets/ModelLoader.h"
#include "engine/Math/Easing.h"

DeathEffectSystem::DeathEffectSystem()
	: isModelLoaded_(false)
{
}

void DeathEffectSystem::Update(NoEngine::ECS::Registry& registry, float deltaTime) {
	// モデルの読み込み（初回のみ）
	if (!isModelLoaded_) {
		NoEngine::Asset::ModelLoader::LoadModel("ParticleModel", "resources/game/td_3105/Model/Particle/Particle.obj");
		isModelLoaded_ = true;
	}

	// DeathParticleComponentを持つエンティティを走査
	auto view = registry.View<DeathParticleComponent, NoEngine::Component::TransformComponent, NoEngine::Component::MaterialComponent>();
	if (view.Empty()) return;

	for (auto entity : view) {
		auto* particle = registry.GetComponent<DeathParticleComponent>(entity);
		auto* transform = registry.GetComponent<NoEngine::Component::TransformComponent>(entity);
		auto* material = registry.GetComponent<NoEngine::Component::MaterialComponent>(entity);

		if (!particle || !transform || !material) continue;

		// タイマー更新
		particle->timer += deltaTime;
		float t = particle->timer / particle->duration;

		// 演出終了判定
		if (t >= 1.0f) {
			registry.DestroyEntity(entity);
			continue;
		}

		// イージングで位置を更新（EaseOutCircを使用）
		transform->translate = NoEngine::Easing::EaseOutCirc(
			particle->startPosition,
			particle->targetPosition,
			t
		);

		// スケールをイージング（少し縮小）
		float scaleT = NoEngine::Easing::EaseOutCubic(1.0f, 0.5f, t);
		float scale = particle->startScale * scaleT;
		transform->scale = NoEngine::Math::Vector3(scale, scale, scale);

		// 透明度をイージング（徐々にフェードアウト）
		float alpha = NoEngine::Easing::EaseOutCubic(1.0f, 0.0f, t);
		material->color = particle->startColor;
		material->color.a = alpha;
	}
}
