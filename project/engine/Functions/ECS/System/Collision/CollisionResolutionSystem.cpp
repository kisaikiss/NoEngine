#include "CollisionResolutionSystem.h"
#include "../../Event/CollisionEvents.h"
#include "../../Component/Common/TransformComponent.h"
#include "../../Component/Common/VelocityComponent.h"

namespace NoEngine {
namespace ECS {
void CollisionResolutionSystem::Update(Registry& registry, float deltaTime) {
	// 前フレームからの経過時間は使用しない
	static_cast<void>(deltaTime);
	// 衝突イベントを取り出す
	auto contactEvent = registry.PollEvent<Event::ContactEvent>();
	if (!contactEvent.has_value()) return;
	for (const auto& contact : contactEvent->contacts) {

		auto* velocityA = registry.GetComponent<Component::VelocityComponent>(contact.a);
		auto* velocityB = registry.GetComponent<Component::VelocityComponent>(contact.b);

		auto* bodyA = registry.GetComponent<Component::CollisionBody>(contact.a);
		auto* bodyB = registry.GetComponent<Component::CollisionBody>(contact.b);

		if (bodyA) {
			if (bodyA->type == Component::BodyType::Dynamic) {
				if (velocityA)Slide(velocityA->linear, contact.normal);
			}
		}

		if (bodyB) {
			if (bodyB->type == Component::BodyType::Dynamic) {
				if (velocityB)Slide(velocityB->linear, contact.normal);

			}
		}
	}
	// 衝突情報をイベントとしてゲームアプリケーション用衝突応答のシステムへ送る
	registry.EmitEvent(contactEvent.value());
}

void CollisionResolutionSystem::Slide(Math::Vector3& velocity, const Math::Vector3& normal) {
	float vn = velocity.Dot(normal);
	if (vn < 0.0f) {
		velocity -= normal * vn;
	}
}
}
}