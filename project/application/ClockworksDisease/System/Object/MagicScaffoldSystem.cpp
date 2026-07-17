#include "stdafx.h"
#include "MagicScaffoldSystem.h"

REFLECT_STRUCT_BEGIN(MagicScaffoldComponent)
REFLECT_FIELD(finalSize)
REFLECT_STRUCT_END(MagicScaffoldComponent);

void MagicScaffoldSystem::Update(No::Registry& registry, float deltaTime) {
	struct ScaffoldInfo{
		No::Entity e = No::INVALID_ENTITY;
		uint32_t generation = 0;
	};
	std::vector<ScaffoldInfo> infos;

	for (auto e : registry.View<No::TransformComponent, MagicScaffoldComponent>()) {
		auto* transform = registry.GetComponent<No::TransformComponent>(e);
		auto* scaffold = registry.GetComponent<MagicScaffoldComponent>(e);

		if (scaffold->generation == 0) {
			static uint32_t generation = 0;
			scaffold->generation = ++generation;
		}
		if (scaffold->isDead) {
			scaffold->time -= deltaTime;
			if (scaffold->time <= 0.0f) {
				scaffold->time = 0.0f;
				registry.DestroyEntity(e);
			}

		} else {
			scaffold->time += deltaTime;
			if (scaffold->time > 1.0f) {
				scaffold->time = 1.0f;
			}

			infos.push_back({ e,scaffold->generation });
		}
		transform->scale = No::EaseInOutBack(No::Vector3::ZERO, scaffold->finalSize, scaffold->time);
		float angle = No::EaseInOutBack(0.0f, PI * 4.f, scaffold->time);
		transform->rotation.FromAxisAngle(No::Vector3::UP, angle);
	}
	
	if (infos.size() > 1) {
		std::sort(infos.begin(), infos.end(),[](const ScaffoldInfo& a, const ScaffoldInfo& b) {
			return a.generation < b.generation;
		});

		for (size_t i = 0; i < infos.size() - 1; i++) {
			registry.GetComponent<MagicScaffoldComponent>(infos[i].e)->isDead = true;
		}
	}

}
