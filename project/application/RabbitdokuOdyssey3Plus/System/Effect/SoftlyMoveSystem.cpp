#include "stdafx.h"
#include "SoftlyMoveSystem.h"
#include "../../Game/RabbitdokuTag.h"

void SoftlyMoveSystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<SoftlyMoveComponent, No::Transform2DComponent, No::Velocity2DComponent>();
	for (auto e : view) {
		auto* v = registry.GetComponent<No::Velocity2DComponent>(e);
		auto* s = registry.GetComponent<SoftlyMoveComponent>(e);
		s->t += deltaTime * s->moveSpeed;
		if (s->t > PI * 2) {
			s->t -= PI * 2;
		}
		v->linear.x = std::cosf(s->t) * s->amplitude.x;
		v->linear.y = std::sinf(s->t) * s->amplitude.y;
	}
}
