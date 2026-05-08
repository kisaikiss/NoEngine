#include "RabbitdokuMoveSystem.h"
#include "../Component/RabbitdokuComponent.h"

void RabbitdokuMoveSystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<Rabbitdoku>();
	(void)deltaTime;
	for (auto e : view) {
		auto* velocity = registry.GetComponent<No::Velocity2DComponent>(e);
		auto* sprite = registry.GetComponent<No::SpriteComponent>(e);
		auto* playerVariables = registry.GetComponent<Rabbitdoku>(e);
		velocity->linear = No::Vector2::ZERO;
		if (No::Keyboard::IsPress(VK_RIGHT)) {
			velocity->linear.x += 500.f;
			sprite->flipX = false;
		}
		if (No::Keyboard::IsPress(VK_LEFT)) {
			velocity->linear.x -= 500.f;
			sprite->flipX = true;
		}


		// 縦移動
		if (No::Keyboard::IsTrigger(VK_LSHIFT)) {
			playerVariables->yVelocity = -playerVariables->jumpSpeed;
		}

		if (No::Keyboard::PreIsTrigger(VK_LSHIFT)) {
			if (playerVariables->yVelocity < 0.f) {
				playerVariables->yVelocity *= 0.45f;
			}
		}

		static const float kGravity = 980.f;


		playerVariables->yVelocity += kGravity * deltaTime;

		velocity->linear.y = playerVariables->yVelocity;

		


	
	}
}
