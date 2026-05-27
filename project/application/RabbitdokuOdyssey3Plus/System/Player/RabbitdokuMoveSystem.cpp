#include "RabbitdokuMoveSystem.h"
#include "../../Component/RabbitdokuComponent.h"
#include "../../Game/RabbitdokuResetEvent.h"
#include "application/RabbitdokuOdyssey3Plus/Game/RabbitdokuSerializer.h"

void RabbitdokuMoveSystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<Rabbitdoku, No::Velocity2DComponent, No::SpriteComponent, No::GroundStateComponent, No::Animator2DComponent>();
	static_cast<void>(deltaTime);
	for (auto e : view) {
		auto* playerVariables = registry.GetComponent<Rabbitdoku>(e);
		if (playerVariables->state == RabbitdokuState::Dead) {
			DeadMove(registry, e, deltaTime);
			continue;
		}
		auto* velocity = registry.GetComponent<No::Velocity2DComponent>(e);
		auto* sprite = registry.GetComponent<No::SpriteComponent>(e);
		auto* groundState = registry.GetComponent<No::GroundStateComponent>(e);
		auto* animator = registry.GetComponent<No::Animator2DComponent>(e);
		playerVariables->nextState = RabbitdokuState::Unknown;


		// 左右移動
		velocity->linear = No::Vector2::ZERO;

		if (groundState->isGrounded) playerVariables->wallJumpTimer = 0.f;
		if (playerVariables->wallJumpTimer > 0.0f) {
			playerVariables->wallJumpTimer -= deltaTime;
			switch (playerVariables->wallJumpDirection) {
			case RabbitdokuDirection::kRight:
				velocity->linear += playerVariables->moveSpeed;
				break;
			case RabbitdokuDirection::kLeft:
				velocity->linear += -playerVariables->moveSpeed;
				break;
			}
		} else {
			playerVariables->wallJumpTimer = 0.0f;
			if (No::InputIsPress("Right")) {
				velocity->linear.x += playerVariables->moveSpeed;
				sprite->flipX = false;
			} else if (No::InputIsPress("Left")) {
				velocity->linear.x -= playerVariables->moveSpeed;
				sprite->flipX = true;
			}
		}


		

		// 横移動していたら次のステートを歩きにする
		if (velocity->linear.x) {
			if (playerVariables->state != RabbitdokuState::Walk)
				playerVariables->nextState = RabbitdokuState::Walk;
		} else {
			if (playerVariables->state != RabbitdokuState::Wait)
				playerVariables->nextState = RabbitdokuState::Wait;
		}

		// 壁に張り付いていたら次のステートを壁にする
		if (playerVariables->sizeCollide && playerVariables->yVelocity) {
			playerVariables->nextState = RabbitdokuState::Wall;
			groundState->isGrounded = true;
		}

		// 縦移動
		if (No::InputIsTrigger("Jump")) {
			if (groundState->isGrounded) {
				playerVariables->yVelocity = -playerVariables->jumpSpeed;
				if (playerVariables->nextState == RabbitdokuState::Wall) {
					const float kWallJumpTime = 0.5f;
					playerVariables->wallJumpTimer = kWallJumpTime;
					if (sprite->flipX) {
						playerVariables->wallJumpDirection = RabbitdokuDirection::kRight;
						sprite->flipX = false;
					} else {
						playerVariables->wallJumpDirection = RabbitdokuDirection::kLeft;
						sprite->flipX = true;
					}
				}
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

		// 移動の速度がある、かつ壁に張り付いていなかったらジャンプ中にする
		if (playerVariables->yVelocity && !playerVariables->sizeCollide) {
			playerVariables->nextState = RabbitdokuState::Jump;
		}

		// 重力
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
			break;
		case RabbitdokuState::Wall:
			playerVariables->state = RabbitdokuState::Wall;
			animator->framesNum = 1;
			animator->currentAnimation = 4;
			sprite->uv.x = 0.f;
			break;
		default:
			break;
		}


		if (No::InputIsTrigger("Reset")) {
			registry.GetComponent<SaveData>(e)->death++;
			registry.GetComponent<SaveData>(e)->totalDeath++;
			RabbitdokuResetEvent dead;
			registry.EmitEvent(dead);
		}

		playerVariables->sizeCollide = false;
		if (velocity->linear.x > 800.f) {
			velocity->linear.x = 800.f;
		} else if (velocity->linear.x < -800.f) {
			velocity->linear.x = -800.f;
		}
	}
}

void RabbitdokuMoveSystem::DeadMove(No::Registry& registry, No::Entity e, float deltaTime) {
	auto* playerVariables = registry.GetComponent<Rabbitdoku>(e);
	auto* velocity = registry.GetComponent<No::Velocity2DComponent>(e);
	auto* sprite = registry.GetComponent<No::SpriteComponent>(e);
	auto* animator = registry.GetComponent<No::Animator2DComponent>(e);

	velocity->linear = No::GetRandomVal(No::Vector2(-150.f, -150.f), No::Vector2(150.f, 150.f));
	animator->framesNum = 1;
	animator->currentAnimation = 5;
	sprite->uv.x = 0.f;
	playerVariables->deadTimer += deltaTime;

	static const float kDeadTime = 0.75f;
	if (playerVariables->deadTimer > kDeadTime) {
		registry.GetComponent<SaveData>(e)->death++;
		registry.GetComponent<SaveData>(e)->totalDeath++;
		RabbitdokuResetEvent dead;
		registry.EmitEvent(dead);
		registry.DestroyEntity(e);
	}
}
