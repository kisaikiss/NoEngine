#include "stdafx.h"
#include "Movement2DSystem.h"
#include "../Component/Transform2DComponent.h"
#include "../Component/VelocityComponent.h"

namespace NoEngine {
namespace ECS {
void Movement2DSystem::Update(Registry& registry, float deltaTime) {
	auto view2d = registry.View<Component::Transform2DComponent, Component::Velocity2DComponent>();
	for (auto entity : view2d) {
		auto* transform = registry.GetComponent<Component::Transform2DComponent>(entity);
		auto* velocity = registry.GetComponent<Component::Velocity2DComponent>(entity);
		switch (axis_) {
		case MovementAxis::Horizontal:
			transform->translate.x += velocity->linear.x * deltaTime;
			break;
		case MovementAxis::Vertical:
			transform->translate.y += velocity->linear.y * deltaTime;
			break;
		case MovementAxis::All:
			transform->translate += velocity->linear * deltaTime;
			break;
		}
		
	}
}
}
}