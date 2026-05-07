#include "RabbitdokuMoveSystem.h"
#include "../Component/RabbitdokuComponent.h"

void RabbitdokuMoveSystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<Rabbitdoku>();
	(void)deltaTime;
	for (auto e : view) {
		auto* velocity = registry.GetComponent<No::Velocity2DComponent>(e);
		velocity->linear = No::Vector2::ZERO;
		if (No::Keyboard::IsPress(VK_RIGHT)) {
			velocity->linear.x += 100.f;
		}
		if (No::Keyboard::IsPress(VK_LEFT)) {
			velocity->linear.x -= 100.f;
		}
	}
}
