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
			auto* ground = registry.GetComponent<Component::GroundStateComponent>(entity);
			const float snapDistance = ground ? ground->snapDistance : 0.f;
			const float minNormalY = ground ? ground->snapMinNormalY : 0.75f;
			const auto moved = Math::MoveCapsuleTerrain(registry, worldCapsule, displacement,
				&velocity->linear, snapDistance, minNormalY);
			transform->SetWorldPosition(registry, transform->GetWorldPosition(registry) + moved);
		} else {
			transform->translate += displacement;
		}
		if (auto* ground = registry.GetComponent<Component::GroundStateComponent>(entity)) {
			ground->snapDistance = 0.f;
		}
	}

}

#include "../SystemManager.h"
REGISTER_SYSTEM(NoEngine::ECS::MovementSystem, "MovementSystem", "Movement")
