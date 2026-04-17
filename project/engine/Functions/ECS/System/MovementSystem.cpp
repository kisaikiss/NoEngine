#include "MovementSystem.h"
#include "../Component/TransformComponent.h"
#include "../Component/VelocityComponent.h"

void NoEngine::ECS::MovementSystem::Update(Registry& registry, float deltaTime) {
	auto view = registry.View<Component::TransformComponent, Component::VelocityComponent>();
	for (auto entity : view) {
		auto* transform = registry.GetComponent<Component::TransformComponent>(entity);
		auto* velocity = registry.GetComponent<Component::VelocityComponent>(entity);
		transform->translate += velocity->linear * deltaTime;
	}
}
