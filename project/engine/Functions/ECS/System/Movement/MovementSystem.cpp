#include "MovementSystem.h"
#include "../../Component/Common/TransformComponent.h"
#include "../../Component/Common/VelocityComponent.h"
#include "engine/Math/Collision/CollisionFanc.h"

void NoEngine::ECS::MovementSystem::Update(Registry& registry, float deltaTime) {
	auto view3d = registry.View<Component::TransformComponent, Component::VelocityComponent>();
	for (auto entity : view3d) {
		auto* transform = registry.GetComponent<Component::TransformComponent>(entity);
		auto* velocity = registry.GetComponent<Component::VelocityComponent>(entity);
		const auto displacement = velocity->linear * deltaTime;
		auto* body = registry.GetComponent<Component::CollisionBody>(entity);
		auto* capsule = registry.GetComponent<Math::CapsuleCollider>(entity);
		if (body && body->type == Component::BodyType::Dynamic && capsule) {
			const auto worldCapsule = GetWorldCapsule(transform, capsule, registry);
			const auto moved = Math::MoveCapsuleTerrain(registry, worldCapsule, displacement, &velocity->linear);
			transform->SetWorldPosition(registry, transform->GetWorldPosition(registry) + moved);
		} else {
			transform->translate += displacement;
		}
	}

}

#include "../SystemManager.h"
REGISTER_SYSTEM(NoEngine::ECS::MovementSystem, "MovementSystem", "Movement")
