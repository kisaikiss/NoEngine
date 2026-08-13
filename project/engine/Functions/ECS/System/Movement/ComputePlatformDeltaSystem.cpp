#include "stdafx.h"
#include "ComputePlatformDeltaSystem.h"
#include "engine/Math/Collision/ColliderComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"

namespace NoEngine {
namespace ECS {
void ComputePlatformDeltaSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto view = registry.View<Component::TransformComponent, Component::PlatformDeltaComponent>();
	for (auto entity : view) {
		auto* delta = registry.GetComponent<Component::PlatformDeltaComponent>(entity);
		auto* transform = registry.GetComponent<Component::TransformComponent>(entity);
		delta->frameDelta = transform->translate - delta->previousTranslate;
	}
}
}
}