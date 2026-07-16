#include "PlayerVerticalVelocitySystem.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"

void PlayerVerticalVelocitySystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<PlayerComponent, No::VelocityComponent, No::GroundStateComponent,
		No::ParticleEmitterComponent, PlayerMoveTransientComponent>();

	for (auto entity : view) {
		auto* playerVariables = registry.GetComponent<PlayerComponent>(entity);
		auto* velocity = registry.GetComponent<No::VelocityComponent>(entity);
		auto* groundState = registry.GetComponent<No::GroundStateComponent>(entity);
		auto* particleEmitter = registry.GetComponent<No::ParticleEmitterComponent>(entity);
		auto* transientState = registry.GetComponent<PlayerMoveTransientComponent>(entity);

		const bool isGrounded = groundState->isGrounded;
		const bool justJumped = transientState->justJumped;

		if (transientState->isAirDashing) {
			// 空中ダッシュ中は重力の影響を受けない。
			// yVelocity はあえて更新せず据え置くことで、ダッシュ終了後に
			// 元の落下/上昇速度からスムーズに重力計算を再開できるようにする。
			velocity->linear.y = 0.f;
			particleEmitter->active = false;
		} else if (isGrounded && !justJumped) {
			// 接地中は重力加算しない。斜面yのみvelocityに反映
			velocity->linear.y = transientState->slopeY;
		} else {
			// 空中 or ジャンプ直後
			playerVariables->yVelocity += playerVariables->gravity * deltaTime;
			velocity->linear.y = playerVariables->yVelocity;
			particleEmitter->active = false;
		}

		playerVariables->groundNormal = No::Vector3::ZERO;
	}
}
