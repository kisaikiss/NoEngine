#include "stdafx.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "application/ClockworksDisease/Component/Player/PlayerDeathComponent.h"
#include "application/ClockworksDisease/Component/Dialogue/DialogueComponent.h"
#include "application/ClockworksDisease/Component/Game/GoalDirectionComponent.h"
#include "application/ClockworksDisease/Component/Camera/FollowCameraComponent.h"
#include "application/ClockworksDisease/Component/Camera/CameraIntroComponent.h"
#include "application/ClockworksDisease/System/Game/CollisionLayer.h"
#include "engine/Math/Collision/CollisionFanc.h"

namespace {
class PlayerAttackHitSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override {
		if (deltaTime <= 0.0f) return;
		for (auto camera : registry.View<No::TransformComponent, No::CameraComponent, FollowCameraComponent>()) {
			if (registry.Has<CameraLockTag>(camera)) return;
		}
		for (auto player : registry.View<PlayerComponent, PlayerActionComponent,
			No::TransformComponent, No::CapsuleCollider>()) {
			const auto* action = registry.GetComponent<PlayerActionComponent>(player);
			if (action->activeAction != PlayerActionType::kSpinAttack) continue;
			if (registry.Has<DialogueLockTag>(player) || registry.Has<GoalDirectionLockTag>(player)) continue;
			const auto* death = registry.GetComponent<PlayerDeathSequenceComponent>(player);
			if (death && death->phase != PlayerDeathSequenceComponent::Phase::kBlackOut
				&& death->phase != PlayerDeathSequenceComponent::Phase::kFadeIn) continue;

			const float radius = registry.GetComponent<PlayerComponent>(player)->spinAttackRadius;
			if (!std::isfinite(radius) || radius <= 0.0f) continue;
			auto attack = NoEngine::GetWorldCapsule(registry.GetComponent<No::TransformComponent>(player),
				registry.GetComponent<No::CapsuleCollider>(player), registry);
			attack.radius = radius;

			// 移動後・死亡判定前の位置で全周を調べ、範囲内の敵をすべて倒す。
			for (auto enemy : registry.View<CollisionLayerComponent, No::TransformComponent, No::CapsuleCollider>()) {
				if (enemy == player) continue;
				auto* layer = registry.GetComponent<CollisionLayerComponent>(enemy);
				if ((layer->layer & CollisionLayerComponent::Enemy) == CollisionLayerComponent::None) continue;
				const auto target = NoEngine::GetWorldCapsule(registry.GetComponent<No::TransformComponent>(enemy),
					registry.GetComponent<No::CapsuleCollider>(enemy), registry);
				if (!NoEngine::Math::TestCapsuleCapsule(attack, target)) continue;

				// 実際の削除はフレーム末尾なので、直後の接触死判定から先に除外する。
				layer->layer = CollisionLayerComponent::None;
				registry.DestroyEntity(enemy);
			}
		}
	}
};
}

REGISTER_SYSTEM(PlayerAttackHitSystem, "PlayerAttackHitSystem", "Gameplay")
