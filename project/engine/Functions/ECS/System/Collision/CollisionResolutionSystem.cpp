#include "CollisionResolutionSystem.h"
#include "../../Event/CollisionEvents.h"
#include "../../Component/Common/TransformComponent.h"
#include "../../Component/Common/VelocityComponent.h"
#include <unordered_map>
#include <vector>

namespace NoEngine {
namespace ECS {

namespace {
// 位置補正を「一気に100%」ではなく緩やかにかける係数。
// 大きいほど即座に解消されるがポップしやすく、小さいほど残留めり込みが増える。
constexpr float kPositionCorrectionPercent = 0.6f;
// これ以下のめり込みは補正しない（常時ジリジリ動いてしまうのを防ぐ）
constexpr float kPenetrationSlop = 0.005f;
// ほぼ同じ方向とみなす閾値（同一面からの重複接触を1つにまとめる）
constexpr float kSameDirectionDot = 0.9f;

struct PendingCorrection {
    // (法線方向, その方向での最大penetration) のリスト
    std::vector<std::pair<Math::Vector3, float>> directional;
};

void AddCorrection(std::unordered_map<Entity, PendingCorrection>& pending,
    Entity entity, const Math::Vector3& normal, float penetration) {
    auto& pc = pending[entity];
    for (auto& [n, p] : pc.directional) {
        if (n.Dot(normal) > kSameDirectionDot) {
            // 同じ面とみなし、大きい方だけ採用（合算しない）
            p = std::max(p, penetration);
            return;
        }
    }
    pc.directional.push_back({ normal, penetration });
}
}

void CollisionResolutionSystem::Update(Registry& registry, float deltaTime) {
    static_cast<void>(deltaTime);
    auto contactEvent = registry.PollEvent<Event::ContactEvent>();
    if (!contactEvent.has_value()) return;

    std::unordered_map<Entity, PendingCorrection> pending;

    // 1パス目：速度のSlideと、補正量の集計（まだtranslateは動かさない）
    for (const auto& contact : contactEvent->contacts) {
        auto* velocityA = registry.GetComponent<Component::VelocityComponent>(contact.a);
        auto* velocityB = registry.GetComponent<Component::VelocityComponent>(contact.b);
        auto* bodyA = registry.GetComponent<Component::CollisionBody>(contact.a);
        auto* bodyB = registry.GetComponent<Component::CollisionBody>(contact.b);

        if (bodyA && bodyA->type == Component::BodyType::Dynamic) {
            if (velocityA) Slide(velocityA->linear, contact.normal);
            if (contact.penetration > 0.0f) {
                AddCorrection(pending, contact.a, contact.normal, contact.penetration);
            }
        }
        if (bodyB && bodyB->type == Component::BodyType::Dynamic) {
            if (velocityB) Slide(velocityB->linear, contact.normal);
            if (contact.penetration > 0.0f) {
                AddCorrection(pending, contact.b, contact.normal * -1.0f, contact.penetration);
            }
        }
    }

    // 2パス目：エンティティごとに、方向別最大値だけを1回ずつ適用
    for (auto& [entity, pc] : pending) {
        auto* transform = registry.GetComponent<Component::TransformComponent>(entity);
        if (!transform) continue;
        for (auto& [normal, pen] : pc.directional) {
            float corrected = std::max(pen - kPenetrationSlop, 0.0f);
            if (corrected <= 0.0f) continue;
            transform->translate += normal * (corrected * kPositionCorrectionPercent);
        }
    }

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