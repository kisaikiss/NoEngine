#include "NarrowPhaseSystem.h"
#include "../../Event/CollisionEvents.h"
#include "../../Component/Common/TransformComponent.h"
#include "../../Component/Common/VelocityComponent.h"

#include "engine/Functions/Renderer/Primitive.h"
#include "engine/Math/MathInclude.h"
#include "engine/Math/Collision/CollisionHelper.h"

namespace NoEngine {
namespace ECS {
using namespace Math;

void NarrowPhaseSystem::Update(Registry& registry, float deltaTime) {
	// 前フレームからの経過時間は使用しない
	static_cast<void>(deltaTime);

	// CollisionResolutionSystemからゲームへ送られるはずのイベントを消費しておく
	// ゲームアプリケーション用衝突応答のシステムが存在しない場合の対策
	registry.PollEvent<Event::ContactEvent>();

	// 衝突情報
	Event::ContactEvent contactEvent;

	// ToDo : カプセルVsその他になっているので、その他Vsその他も衝突判定するようにする
	auto aabbView = registry.View<Math::AABBCollider, Component::TransformComponent>();
	auto obbView = registry.View<Math::OBBCollider, Component::TransformComponent>();
	auto sphereView = registry.View<Math::SphereCollider, Component::TransformComponent>();
	auto terrainView = registry.View<Math::TerrainMesh>();
	auto capsuleView = registry.View<Math::CapsuleCollider, Component::TransformComponent>();

	for (auto capsuleE : capsuleView) {
		auto* capsuleTransform = registry.GetComponent<Component::TransformComponent>(capsuleE);
		auto* capsuleCollider = registry.GetComponent<Math::CapsuleCollider>(capsuleE);
		for (auto boxE : aabbView) {
			auto* boxTransform = registry.GetComponent<Component::TransformComponent>(boxE);
			auto* boxCollider = registry.GetComponent<Math::AABBCollider>(boxE);

			// 衝突判定を行う
			auto collide = Math::TestCapsuleAABB(capsuleTransform, capsuleCollider, boxTransform, boxCollider, registry);

			if (!collide.hit) continue;

			// 衝突情報を格納
			Contact contact;
			contact.a = capsuleE;
			contact.b = boxE;
			contact.normal = collide.normal;
			contact.penetration = collide.penetration;
			contact.contactPosition = ClassifyContact(collide.normal);

			contactEvent.contacts.push_back(contact);
		}

		for (auto boxE : obbView) {
			auto* boxTransform = registry.GetComponent<Component::TransformComponent>(boxE);
			auto* boxCollider = registry.GetComponent<Math::OBBCollider>(boxE);

			// 衝突判定を行う
			auto collide = Math::TestCapsuleOBB(capsuleTransform, capsuleCollider, boxTransform, boxCollider, registry);

			if (!collide.hit) continue;

			// 衝突情報を格納
			Contact contact;
			contact.a = capsuleE;
			contact.b = boxE;
			contact.normal = collide.normal;
			contact.penetration = collide.penetration;
			contact.contactPosition = ClassifyContact(collide.normal);

			contactEvent.contacts.push_back(contact);
		}

		for (auto sphereE : sphereView) {
			auto* sphereTransform = registry.GetComponent<Component::TransformComponent>(sphereE);
			auto* sphereCollider = registry.GetComponent<Math::SphereCollider>(sphereE);

			// 衝突判定を行う
			auto collide = Math::TestCapsuleSphere(capsuleTransform, capsuleCollider, sphereTransform, sphereCollider, registry);

			if (!collide.hit) continue;

			// 衝突情報を格納
			Contact contact;
			contact.a = capsuleE;
			contact.b = sphereE;
			contact.normal = collide.normal;
			contact.penetration = collide.penetration;
			contact.contactPosition = ClassifyContact(collide.normal);

			contactEvent.contacts.push_back(contact);
		}

		// カプセルのワールドAABBを一度だけ計算
		Math::AABBCollider capsuleBounds = ComputeCapsuleWorldBounds(capsuleTransform, capsuleCollider, registry);

		for (auto terrainE : terrainView) {
			auto* terrain = registry.GetComponent<Math::TerrainMesh>(terrainE);

			candidateIndices_.clear();
			Math::QueryBVH(terrain->bvhRoot.get(), capsuleBounds, terrain->triangles, candidateIndices_);

			for (int idx : candidateIndices_) {
				const auto& triangle = terrain->triangles[idx];
				auto collide = Math::TestCapsuleTriangle(capsuleTransform, capsuleCollider, triangle, registry);
				if (!collide.hit) continue;

				Contact contact;
				contact.a = capsuleE;
				contact.b = terrainE;
				contact.normal = collide.normal;
				contact.penetration = collide.penetration;
				contact.contactPosition = ClassifyContact(collide.normal);

				contactEvent.contacts.push_back(contact);
			}
		}
	}
	if (!contactEvent.contacts.empty()) {
		// 衝突情報をイベントとして次のシステムへ送る
		registry.EmitEvent(contactEvent);
	}

}

ContactPosition NarrowPhaseSystem::ClassifyContact(const Math::Vector3& normal) {
	if (normal.y > 0.5f) return ContactPosition::UP;
	if (normal.y < -0.5f) return ContactPosition::DOWN;
	return ContactPosition::SIDE;
}
}
}