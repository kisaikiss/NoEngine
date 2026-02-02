#pragma once
#include "engine/NoEngine.h"

class BatControlSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	void GenerateUpdate(No::Entity entity, No::Registry& registry, float deltaTime);
	void LiveUpdate(No::Entity entity, No::Registry& registry, float deltaTime);
	void DeadUpdate(No::Entity entity, No::Registry& registry, float deltaTime);

	void GenerateSmokeEffect(No::Registry& registry, NoEngine::Vector3 position);
};

