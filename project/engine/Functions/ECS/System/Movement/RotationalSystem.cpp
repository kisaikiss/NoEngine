#include "stdafx.h"
#include "RotationalSystem.h"

#include "../../Component/Common/TransformComponent.h"
#include "../../Component/Common/RotationalVelocityComponent.h"

void NoEngine::ECS::RotationalSystem::Update(Registry& registry, float deltaTime) {
	auto view3d = registry.View<Component::TransformComponent, Component::RotationalVelocityComponent>();
	for (auto entity : view3d) {
		auto* transform = registry.GetComponent<Component::TransformComponent>(entity);
		auto* velocity = registry.GetComponent<Component::RotationalVelocityComponent>(entity);
		const Math::Vector3 rotationDelta = velocity->linear * deltaTime;

		// `linear` は、オイラーの X、Y、および Z 軸周りの角速度（ラジアン/秒）です。
		// ローカル空間における増分回転を構築し、それを現在の向きに適用します。
		Math::Matrix4x4 rotationMatrix;
		rotationMatrix.MakeRotate(rotationDelta);
		Math::Quaternion deltaRotation(rotationMatrix);
		transform->rotation = transform->rotation * deltaRotation;
		transform->rotation.Normalize();
	}

}

#include "../SystemManager.h"
REGISTER_SYSTEM(NoEngine::ECS::RotationalSystem, "RotationalSystem", "Movement")
