#include "PlayerPushBackSystem.h"
#include "../Game/CollisionEvents.h"
#include "../../Component/Player/PlayerComponent.h"

void PlayerPushBackSystem::Update(No::Registry& registry, float deltaTime) {
    static_cast<void>(deltaTime);
    auto playerView = registry.View<PlayerComponent, No::GroundStateComponent>();
    auto view = registry.PollAllEvents<PlayerPushBackEvent>();

    // このフレームで一番確からしい接地normalを選ぶための一時マップ
    std::unordered_map<No::Entity, float> bestPenetration; // player entity -> 最大penetration

    for (const auto& event : view) {
        if (event.position == No::ContactPosition::UP) {
            auto* transform = registry.GetComponent<No::TransformComponent>(event.player);
            auto* ground = registry.GetComponent<No::GroundStateComponent>(event.player);

            ground->isGrounded = true;
            ground->groundHeight = transform->GetWorldPosition(registry).y;

            auto* player = registry.GetComponent<PlayerComponent>(event.player);

            if (player->yVelocity < 0.f) {
                if (!ground->preIsGrounded && player->yVelocity < -3.5f) {
                    registry.AddComponent<No::EffectEmitTag>(event.player);
                }
                player->yVelocity = 0.f;
            }

            float pen = event.penetration;
            auto it = bestPenetration.find(event.player);
            if (it == bestPenetration.end() || pen > it->second) {
                bestPenetration[event.player] = pen;
                player->groundNormal = event.normal;
            }
        }
        if (event.position == No::ContactPosition::DOWN) {
            auto* player = registry.GetComponent<PlayerComponent>(event.player);
            if (player->yVelocity > 0.f) player->yVelocity = 0.f;
        }
    }

    for (auto entity : playerView) {
        auto* player = registry.GetComponent<PlayerComponent>(entity);
        auto* ground = registry.GetComponent<No::GroundStateComponent>(entity);
        if (!ground->isGrounded) {
            player->groundNormal = No::Vector3::UP;
        }
    }
}