#include "CollisionResolutionSystem.h"
#include "../Event/CollisionEvents.h"
#include "../Component/TransformComponent.h"
#include "../Component/VelocityComponent.h"

namespace NoEngine {
namespace ECS {
void CollisionResolutionSystem::Update(Registry& registry, float deltaTime) {
	// 前フレームからの経過時間は使用しない
	static_cast<void>(deltaTime);
	// 衝突イベントを取り出す
	auto contactEvent = registry.PollEvent<Event::ContactEvent>();
	if (!contactEvent.has_value()) return;
	for (const auto& contact : contactEvent->contacts) {
		auto* transform = registry.GetComponent<Component::TransformComponent>(contact.a);

		transform->translate += contact.normal * contact.penetration;
	}
}
}
}