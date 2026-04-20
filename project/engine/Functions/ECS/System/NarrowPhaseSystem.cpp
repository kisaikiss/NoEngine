#include "NarrowPhaseSystem.h"
#include "../Event/CollisionEvents.h"
#include "../Component/TransformComponent.h"
#include "../Component/VelocityComponent.h"

#include "engine/Functions/Renderer/Primitive.h"
#include "engine/Math/MathInclude.h"

namespace NoEngine {
namespace ECS {
void NarrowPhaseSystem::Update(Registry& registry, float deltaTime) {
	// 前フレームからの経過時間は使用しない
	static_cast<void>(deltaTime);

	// 衝突情報
	Event::ContactEvent contactEvent;

	// BroadPhaseがないためとりあえず必要なAABB vs Capsuleのみ衝突判定を行う
	auto boxView = registry.View<Math::AABBCollider, Component::TransformComponent>();
	auto capsuleView = registry.View<Math::CapsuleCollider, Component::TransformComponent>();

	for (auto capsuleE : capsuleView) {
		auto* capsuleTransform = registry.GetComponent<Component::TransformComponent>(capsuleE);
		auto* capsuleCollider = registry.GetComponent<Math::CapsuleCollider>(capsuleE);

		for (auto boxE : boxView) {
			auto* boxTransform = registry.GetComponent<Component::TransformComponent>(boxE);
			auto* boxCollider = registry.GetComponent<Math::AABBCollider>(boxE);
			
			// 衝突判定を行う
			auto collide = Math::TestCapsuleAABB(capsuleTransform, capsuleCollider, boxTransform, boxCollider);
			
			if (!collide.hit) continue;

			// 衝突情報を格納
			Contact contact;
			contact.a = capsuleE;
			contact.b = boxE;
			contact.normal = collide.normal;
			contact.penetration = collide.penetration;

			Primitive::DrawSphere(capsuleCollider->localP0 + capsuleTransform->GetWorldPosition(), capsuleCollider->radius, Math::Color::WHITE);
			Primitive::DrawSphere(capsuleCollider->localP1 + capsuleTransform->GetWorldPosition(), capsuleCollider->radius, Math::Color::WHITE);

			contactEvent.contacts.push_back(contact);
		}
	}
	if (!contactEvent.contacts.empty()) {
		// 衝突情報をイベントとして次のシステムへ送る
		registry.EmitEvent(contactEvent);
	}

}
}
}
