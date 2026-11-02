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
        auto* player = registry.GetComponent<PlayerComponent>(event.player);
        auto* transient = registry.GetComponent<PlayerMoveTransientComponent>(event.player);
        // 衝突判定は移動前の位置で行われるため、踏み切り直後も足元に接触が残る。
        // ジャンプ開始フレームと上昇中は、その接触を着地として扱わない。
        const bool isTakingOff = (transient && transient->justJumped) || player->yVelocity > 0.f;

        const bool isWalkableGround =
            !isTakingOff &&
            event.position == No::ContactPosition::UP &&
            event.normal.y >= player->minGroundNormalY;

        if (isWalkableGround) {
            auto* transform = registry.GetComponent<No::TransformComponent>(event.player);
            auto* ground = registry.GetComponent<No::GroundStateComponent>(event.player);

            ground->isGrounded = true;
            ground->groundHeight = transform->GetWorldPosition(registry).y;
            ground->groundEntity = event.platform;

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

REGISTER_SYSTEM(::PlayerPushBackSystem, "PlayerPushBackSystem", "Gameplay")
