#include "stdafx.h"
#include "PlayerAnimationSystem.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "PlayerStateMachine.h"

void PlayerAnimationSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto view = registry.View<PlayerComponent, No::AnimatorComponent>();

	for (auto entity : view) {
		auto* playerVariables = registry.GetComponent<PlayerComponent>(entity);
		auto* animator = registry.GetComponent<No::AnimatorComponent>(entity);
		const auto* action = registry.GetComponent<PlayerActionComponent>(entity);
		// 攻撃の単発再生は移動状態のアニメーションで上書きしない。
		if (action && action->activeAction == PlayerActionType::kSpinAttack) continue;
		animator->loop = true;

		const bool moving = false;
		PlayerStateContext stateContext{ *playerVariables, false, moving, false };
		animator->SetAnimation(PlayerStates::Get(playerVariables->state).AnimationName(stateContext));

	}

}

REGISTER_SYSTEM(::PlayerAnimationSystem, "PlayerAnimationSystem", "Gameplay")
