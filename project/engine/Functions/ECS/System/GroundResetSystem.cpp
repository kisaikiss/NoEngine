#include "stdafx.h"
#include "GroundResetSystem.h"
#include "engine/Math/Collision/ColliderComponent.h"

namespace NoEngine {
namespace ECS {
void GroundResetSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto view = registry.View<Component::GroundStateComponent>();
	for (auto entity : view) {
		auto* groundState = registry.GetComponent<Component::GroundStateComponent>(entity);
		groundState->preIsGrounded = groundState->isGrounded;
		groundState->isGrounded = false;
		groundState->groundHeight = 0.f;
	}
}
}
}