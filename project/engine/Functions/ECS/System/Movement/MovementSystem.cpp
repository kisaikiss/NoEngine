#include "MovementSystem.h"
#include "../../Component/Common/TransformComponent.h"
#include "../../Component/Common/VelocityComponent.h"

void NoEngine::ECS::MovementSystem::Update(Registry& registry, float deltaTime) {
	auto view3d = registry.View<Component::TransformComponent, Component::VelocityComponent>();
	for (auto entity : view3d) {
		auto* transform = registry.GetComponent<Component::TransformComponent>(entity);
		auto* velocity = registry.GetComponent<Component::VelocityComponent>(entity);
		transform->translate += velocity->linear * deltaTime;
	}

}
