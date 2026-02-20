#pragma once
#include "WhiteRadishControlSystem.h"

class CarrotControlSystem : public WhiteRadishControlSystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	void LiveUpdate(No::Entity entity, No::Registry& registry, float deltaTime) override;
	void Shoot(No::Registry& registry, No::TransformComponent* transform);
};

