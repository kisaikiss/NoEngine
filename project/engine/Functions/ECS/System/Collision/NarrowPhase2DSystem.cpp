#include "stdafx.h"
#include "NarrowPhase2DSystem.h"
#include "../../Component/Transform2DComponent.h"

namespace NoEngine {
namespace ECS {
using namespace Math;

void NarrowPhase2DSystem::Update(Registry& registry, float deltaTime) {
	// 前フレームからの経過時間は使用しない
	static_cast<void>(deltaTime);

	// CollisionResolutionSystemからゲームへ送られるはずのイベントを消費しておく
	// ゲームアプリケーション用衝突応答のシステムが存在しない場合の対策
	registry.PollEvent<Event::Contact2DEvent>();

	// 衝突情報
	Event::Contact2DEvent contactEvent;

	// BroadPhaseがないためとりあえず必要なAABB vs AABBのみ衝突判定を行う
    auto boxView = registry.View<Math::AABBCollider2D, Component::Transform2DComponent>();
    std::vector<Entity> boxes;
    for (auto e : boxView) {
        boxes.push_back(e);
    }

    for (size_t i = 0; i < boxes.size(); i++) {
        Entity boxA = boxes[i];
        auto* boxATransform = registry.GetComponent<Component::Transform2DComponent>(boxA);
        auto* boxACollider = registry.GetComponent<Math::AABBCollider2D>(boxA);

        for (size_t j = i + 1; j < boxes.size(); j++) {
            Entity boxB = boxes[j];
            auto* boxBTransform = registry.GetComponent<Component::Transform2DComponent>(boxB);
            auto* boxBCollider = registry.GetComponent<Math::AABBCollider2D>(boxB);


            Math::Collision2D collide;

            switch (axis_) {
            case NoEngine::ECS::NarrowPhase2DSystem::TestAxis::Horizontal:
                collide = Math::TestAABB2DHorizontal(boxATransform, boxACollider, boxBTransform, boxBCollider);
                break;
            case NoEngine::ECS::NarrowPhase2DSystem::TestAxis::Vertical:
                collide = Math::TestAABB2DVertical(boxATransform, boxACollider, boxBTransform, boxBCollider);
                break;
            case NoEngine::ECS::NarrowPhase2DSystem::TestAxis::All:
                collide = Math::TestAABB2D(boxATransform, boxACollider, boxBTransform, boxBCollider);
                break;
            }

            if (!collide.hit) continue;

            Contact2D contact;
            contact.a = boxA;
            contact.b = boxB;
            contact.normal = collide.normal;
            contact.penetration = collide.penetration;
            contact.contactPosition = ClassifyContact(collide.normal);

            contactEvent.contacts.push_back(contact);
        }
    }

	if (!contactEvent.contacts.empty()) {
		// 衝突情報をイベントとして次のシステムへ送る
		registry.EmitEvent(contactEvent);
	}

}

ContactPosition NarrowPhase2DSystem::ClassifyContact(const Math::Vector2& normal) {
	if (normal.y > 0.5f) return ContactPosition::DOWN;
	if (normal.y < -0.5f) return ContactPosition::UP;
	return ContactPosition::SIDE;
}

}
}
