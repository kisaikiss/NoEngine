#include "stdafx.h"
#include "Rotational2DSystem.h"

#include "../../Component/Common/Transform2DComponent.h"
#include "../../Component/Common/RotationalVelocityComponent.h"

void NoEngine::ECS::Rotational2DSystem::Update(Registry& registry, float deltaTime) {

	auto view2d = registry.View<Component::Transform2DComponent, Component::RotationalVelocity2DComponent>();
	for (auto entity : view2d) {
		auto* transform = registry.GetComponent<Component::Transform2DComponent>(entity);
		auto* velocity = registry.GetComponent<Component::RotationalVelocity2DComponent>(entity);
		transform->rotation += velocity->linear * deltaTime;

	}
}

#include "../SystemManager.h"
REGISTER_SYSTEM(NoEngine::ECS::Rotational2DSystem, "Rotational2DSystem", "Movement")
