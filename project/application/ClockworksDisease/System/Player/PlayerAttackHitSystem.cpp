#include "stdafx.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "application/ClockworksDisease/Component/Player/PlayerDeathComponent.h"
#include "application/ClockworksDisease/Component/Dialogue/DialogueComponent.h"
#include "application/ClockworksDisease/Component/Game/GoalDirectionComponent.h"
#include "application/ClockworksDisease/Component/Camera/FollowCameraComponent.h"
#include "application/ClockworksDisease/Component/Camera/CameraIntroComponent.h"
#include "application/ClockworksDisease/System/Game/CollisionLayer.h"
#include "application/ClockworksDisease/System/Game/CollisionEvents.h"

namespace {
class PlayerAttackHitSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override {
		const auto hits = registry.PollAllEvents<PlayerAttackHitEvent>();
		if (deltaTime <= 0.0f) return;
		for (auto camera : registry.View<No::TransformComponent, No::CameraComponent, FollowCameraComponent>()) {
			if (registry.Has<CameraLockTag>(camera)) return;
		}
		for (const auto& hit : hits) {
			const auto player = hit.player;
			const auto* action = registry.GetComponent<PlayerActionComponent>(player);
			if (!action || action->activeAction != PlayerActionType::kSpinAttack) continue;
			if (registry.Has<DialogueLockTag>(player) || registry.Has<GoalDirectionLockTag>(player)) continue;
			const auto* death = registry.GetComponent<PlayerDeathSequenceComponent>(player);
			if (death && death->phase != PlayerDeathSequenceComponent::Phase::kBlackOut
				&& death->phase != PlayerDeathSequenceComponent::Phase::kFadeIn) continue;

			auto* layer = registry.GetComponent<CollisionLayerComponent>(hit.enemy);
			if (!layer || (layer->layer & CollisionLayerComponent::Enemy) == CollisionLayerComponent::None) continue;
			// 実際の削除はフレーム末尾なので、直後の接触死判定から先に除外する。
			layer->layer = CollisionLayerComponent::None;
			registry.DestroyEntity(hit.enemy);
		}
	}
};
}

REGISTER_SYSTEM(PlayerAttackHitSystem, "PlayerAttackHitSystem", "Gameplay")
