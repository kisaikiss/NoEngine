#include "stdafx.h"
#include "CapturePlatformPrevTransformSystem.h"
#include "engine/Math/Collision/ColliderComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"

namespace NoEngine {
namespace ECS {
void CapturePlatformPrevTransformSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto view = registry.View<Component::TransformComponent, Component::PlatformDeltaComponent>();
	for (auto entity : view) {
		auto* delta = registry.GetComponent<Component::PlatformDeltaComponent>(entity);
		auto* transform = registry.GetComponent<Component::TransformComponent>(entity);
		delta->previousTranslate = transform->translate;
	}
}
}
}