#include "MovementSystem.h"
#include "../Component/TransformComponent.h"
#include "../Component/Transform2DComponent.h"
#include "../Component/VelocityComponent.h"

void NoEngine::ECS::MovementSystem::Update(Registry& registry, float deltaTime) {
	auto view3d = registry.View<Component::TransformComponent, Component::VelocityComponent>();
	for (auto entity : view3d) {
		auto* transform = registry.GetComponent<Component::TransformComponent>(entity);
		auto* velocity = registry.GetComponent<Component::VelocityComponent>(entity);
		transform->translate += velocity->linear * deltaTime;
	}

	auto view2d = registry.View<Component::Transform2DComponent, Component::Velocity2DComponent>();
	for (auto entity : view2d) {
		auto* transform = registry.GetComponent<Component::Transform2DComponent>(entity);
		auto* velocity = registry.GetComponent<Component::Velocity2DComponent>(entity);
		transform->translate += velocity->linear * deltaTime;
	}
}
