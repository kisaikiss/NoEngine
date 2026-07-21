#include "stdafx.h"
#include "CollisionResolution2DSystem.h"
#include "../../Event/CollisionEvents.h"
#include "../../Component/Common/Transform2DComponent.h"
#include "../../Component/Common/VelocityComponent.h"

namespace NoEngine {
namespace ECS {

void CollisionResolution2DSystem::Update(Registry& registry, float deltaTime) {
    static_cast<void>(deltaTime);
    using namespace Component;

    auto contactEvent = registry.PollEvent<Event::Contact2DEvent>();
    if (!contactEvent.has_value()) return;

    struct PushInfo {
        Math::Vector2 normal;
        float penetration;
    };

    std::unordered_map<Entity, std::vector<PushInfo>> pushes;

    // 衝突ごとに push 情報を蓄積
    for (auto& c : contactEvent->contacts) {
        Math::Vector2 n = c.normal;
        float p = c.penetration;

        if (p == 0.0f) continue;

        auto* bodyA = registry.GetComponent<CollisionBody>(c.a);
        if (bodyA && bodyA->type == BodyType::Dynamic) {
            pushes[c.a].push_back({ n, p });
        }

        auto* bodyB = registry.GetComponent<CollisionBody>(c.b);
        if (bodyB && bodyB->type == BodyType::Dynamic) {
            pushes[c.b].push_back({ -n, p });
        }
    }

    // Entity ごとに押し戻しを決定
    for (auto& [entity, list] : pushes) {
        Math::Vector2 finalPush = { 0,0 };

        // 最大投影だけ使う
        for (auto& info : list) {
            float proj = finalPush.Dot(info.normal);
            float needed = info.penetration - proj;

            if (needed > 0) {
                finalPush += info.normal * needed;
            }
        }

        // Transform に 1 回だけ適用
        auto* t = registry.GetComponent<Transform2DComponent>(entity);
        t->translate += finalPush;

        // スライド処理
        auto* v = registry.GetComponent<Velocity2DComponent>(entity);
        if (v) {
            Math::Vector2 n = finalPush.Normalize();
            float vn = v->linear.Dot(n);
            if (vn < 0) v->linear -= n * vn;
        }
    }


    // ゲーム側へイベント送信
    registry.EmitEvent(contactEvent.value());
}

void CollisionResolution2DSystem::Slide(Math::Vector2& velocity, const Math::Vector2& normal) {
	float vn = velocity.Dot(normal);
	if (vn < 0.0f) {
		velocity -= normal * vn;
	}
}

}
}
