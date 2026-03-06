#include "SmokeEffectControlSystem.h"
#include "application/TD2_3/Component/EffectComponent.h"
#include "application/TD2_3/tag.h"

void SmokeEffectControlSystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<SmokeEffectComponent>();

	for (auto entity : view) {
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		auto* smoke = registry.GetComponent<SmokeEffectComponent>(entity);
		transform->translate += smoke->velocity * deltaTime;
		smoke->t += deltaTime;
		if (smoke->t > 1.f) smoke->t = 1.f;
		transform->scale = No::EaseOutCirc<No::Vector3>(No::Vector3(5.f, 5.f, 5.f), No::Vector3(0.f, 0.f, 0.f), smoke->t);

		if (transform->scale.x <= 0.f) {
			auto* death = registry.GetComponent<DeathFlag>(entity);
			death->isDead = true;
		}

	}

}
