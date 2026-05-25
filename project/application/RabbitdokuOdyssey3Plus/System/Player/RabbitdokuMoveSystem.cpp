#include "RabbitdokuMoveSystem.h"
#include "../../Component/RabbitdokuComponent.h"

void RabbitdokuMoveSystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<Rabbitdoku, No::Velocity2DComponent, No::SpriteComponent, No::GroundStateComponent, No::Animator2DComponent>();
	static_cast<void>(deltaTime);
	for (auto e : view) {
		auto* velocity = registry.GetComponent<No::Velocity2DComponent>(e);
		auto* sprite = registry.GetComponent<No::SpriteComponent>(e);
		auto* playerVariables = registry.GetComponent<Rabbitdoku>(e);
		auto* groundState = registry.GetComponent<No::GroundStateComponent>(e);
		auto* animator = registry.GetComponent<No::Animator2DComponent>(e);
		playerVariables->nextState = RabbitdokuState::Unknown;

		velocity->linear = No::Vector2::ZERO;
		if (No::InputIsPress("Right")) {
			velocity->linear.x += playerVariables->moveSpeed;
			sprite->flipX = false;
		} else if (No::InputIsPress("Left")) {
			velocity->linear.x -= playerVariables->moveSpeed;
			sprite->flipX = true;
		}

		if (velocity->linear.x) {
			if (playerVariables->state != RabbitdokuState::Walk)
				playerVariables->nextState = RabbitdokuState::Walk;
		} else {
			if (playerVariables->state != RabbitdokuState::Wait)
				playerVariables->nextState = RabbitdokuState::Wait;
		}

		// 縦移動
		if (No::InputIsTrigger("Jump")) {
			if (groundState->isGrounded) {
				playerVariables->yVelocity = -playerVariables->jumpSpeed;
			} else {
				if (playerVariables->canDoubleJump) {
					playerVariables->yVelocity = -playerVariables->doubleJumpSpeed;
					playerVariables->canDoubleJump = false;
				}
			}

		}


		// 上昇中にキーを離したら速度を半減
		if (No::InputIsRelease("Jump")) {
			if (playerVariables->yVelocity < 0.f) {
				playerVariables->yVelocity *= 0.45f;
			}
		}

		if (playerVariables->yVelocity) {
			playerVariables->nextState = RabbitdokuState::Jump;
		}

		static const float kGravity = 9.8f;


		playerVariables->yVelocity += kGravity;

		velocity->linear.y = playerVariables->yVelocity;

		switch (playerVariables->nextState) {
		case RabbitdokuState::Wait:
			playerVariables->state = RabbitdokuState::Wait;
			animator->framesNum = 2;
			animator->currentAnimation = 0;
			animator->frameByFrameTime = 0.3f;
			sprite->uv.x = 0.f;
			break;
		case RabbitdokuState::Walk:
			playerVariables->state = RabbitdokuState::Walk;
			animator->framesNum = 6;
			animator->currentAnimation = 1;
			animator->frameByFrameTime = 0.1f;
			sprite->uv.x = 0.f;
			break;
		case RabbitdokuState::Jump:
			playerVariables->state = RabbitdokuState::Jump;
			animator->framesNum = 1;
			animator->currentAnimation = 2;
			sprite->uv.x = 0.f;
		case RabbitdokuState::Wall:
			playerVariables->state = RabbitdokuState::Jump;
			animator->framesNum = 1;
			animator->currentAnimation = 2;
			sprite->uv.x = 0.f;
		default:
			break;
		}



	}
}
