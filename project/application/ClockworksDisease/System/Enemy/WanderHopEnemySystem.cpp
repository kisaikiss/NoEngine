#include "stdafx.h"
#include "../../Component/Enemy/WanderHopEnemyComponent.h"
#include "../../Component/Player/PlayerComponent.h"

namespace {
using Phase = WanderHopEnemyStateComponent::Phase;

No::Vector3 Horizontal(No::Vector3 vector) {
	vector.y = 0.0f;
	return vector;
}

No::Vector3 MoveToward(const No::Vector3& offset, float speed, float stopDistance, float deltaTime) {
	const float distance = offset.Length();
	if (distance <= stopDistance || distance <= 1e-5f) return No::Vector3::ZERO;
	return offset * (std::min(speed, (distance - stopDistance) / deltaTime) / distance);
}

class WanderHopEnemySystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override {
		if (deltaTime <= 0.0f) return;

		const No::TransformComponent* player = nullptr;
		for (auto entity : registry.View<PlayerComponent, No::TransformComponent>()) {
			player = registry.GetComponent<No::TransformComponent>(entity);
		}
		const auto playerPosition = player ? player->GetWorldPosition(registry) : No::Vector3::ZERO;

		for (auto entity : registry.View<WanderHopEnemyComponent, No::TransformComponent,
			No::VelocityComponent, No::GroundStateComponent>()) {
			auto* settings = registry.GetComponent<WanderHopEnemyComponent>(entity);
			auto* transform = registry.GetComponent<No::TransformComponent>(entity);
			auto* velocity = registry.GetComponent<No::VelocityComponent>(entity);
			auto* ground = registry.GetComponent<No::GroundStateComponent>(entity);
			auto* state = registry.GetComponent<WanderHopEnemyStateComponent>(entity);
			if (!state) state = registry.AddComponent<WanderHopEnemyStateComponent>(entity);

			const auto position = transform->GetWorldPosition(registry);
			if (!state->initialized) {
				state->spawnPosition = position;
				state->initialized = true;
			}
			state->justJumped = false;

			const auto toPlayer = playerPosition - position;
			const auto horizontalToPlayer = Horizontal(toPlayer);
			const float playerDistance = horizontalToPlayer.Length();
			const bool playerWithinHeight = player && std::abs(toPlayer.y) <= settings->maxHeightDifference;
			const bool detected = playerWithinHeight && playerDistance <= settings->detectionDistance;
			const bool lost = !playerWithinHeight || playerDistance > settings->loseDistance;

			// 接地判定は前回の更新結果を使い、踏み切り時は直後の接触を無視する。
			if (state->phase == Phase::Wander && ground->isGrounded && detected) {
				state->phase = Phase::AlertHop;
				state->justJumped = true;
				ground->isGrounded = false;
				velocity->linear.y = settings->jumpSpeed;
			} else if (state->phase == Phase::AlertHop && ground->isGrounded) {
				state->phase = Phase::Chase;
			}
			if (state->phase == Phase::Chase && lost) {
				state->phase = Phase::Wander;
				state->wanderTime = 0.0f;
			}

			No::Vector3 horizontalVelocity = No::Vector3::ZERO;
			if (state->phase == Phase::Wander && ground->isGrounded) {
				state->wanderTime -= deltaTime;
				if (state->wanderTime <= 0.0f) {
					const float angle = std::uniform_real_distribution<float>(0.0f, 2.0f * std::numbers::pi_v<float>)(random_);
					const float radius = std::uniform_real_distribution<float>(0.5f, 1.0f)(random_) * settings->wanderRadius;
					state->wanderTarget = state->spawnPosition +
						No::Vector3{std::cos(angle) * radius, 0.0f, std::sin(angle) * radius};
					state->wanderTime = std::uniform_real_distribution<float>(1.0f, 2.0f)(random_);
				}
				horizontalVelocity = MoveToward(Horizontal(state->wanderTarget - position),
					settings->walkSpeed, 0.0f, deltaTime);
			} else if (state->phase == Phase::Chase && ground->isGrounded) {
				horizontalVelocity = MoveToward(horizontalToPlayer, settings->chaseSpeed, 0.85f, deltaTime);
			}

			const auto facing = state->phase == Phase::AlertHop && player ? horizontalToPlayer : horizontalVelocity;
			if (facing.LengthSquared() > 1e-6f) {
				transform->rotation.LookRotation(facing, No::Vector3::UP);
			}
			velocity->linear.x = horizontalVelocity.x;
			velocity->linear.z = horizontalVelocity.z;
			if (ground->isGrounded) {
				velocity->linear.y = 0.0f;
				ground->snapDistance = std::min(0.25f, horizontalVelocity.Length() * deltaTime + 0.002f);
			} else if (!state->justJumped) {
				constexpr float kGravity = -18.0f;
				constexpr float kMaxFallSpeed = -20.0f;
				velocity->linear.y = std::max(kMaxFallSpeed, velocity->linear.y + kGravity * deltaTime);
			}
		}
	}

private:
	std::mt19937 random_{std::random_device{}()};
};
}

REFLECT_STRUCT_BEGIN(WanderHopEnemyComponent, "Application")
REFLECT_FIELD(walkSpeed),
REFLECT_FIELD(chaseSpeed),
REFLECT_FIELD(wanderRadius),
REFLECT_FIELD(detectionDistance),
REFLECT_FIELD(loseDistance),
REFLECT_FIELD(maxHeightDifference),
REFLECT_FIELD(jumpSpeed)
REFLECT_STRUCT_END(WanderHopEnemyComponent)

REGISTER_SYSTEM(WanderHopEnemySystem, "WanderHopEnemySystem", "Gameplay")
