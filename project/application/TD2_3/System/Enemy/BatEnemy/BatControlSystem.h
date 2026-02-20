#pragma once
#include "engine/NoEngine.h"

class BatControlSystem : public No::ISystem {
public:
	virtual void Update(No::Registry& registry, float deltaTime) override;
protected:
	void GenerateUpdate(No::Entity entity, No::Registry& registry, float deltaTime);
	virtual void LiveUpdate(No::Entity entity, No::Registry& registry, float deltaTime);
	void DeadUpdate(No::Entity entity, No::Registry& registry, float deltaTime);

	void GenerateSmokeEffect(No::Registry& registry, No::Vector3 position);

	virtual void CheckCollideEntity(No::Registry& registry, No::Entity enemyEntity);
private:
	bool isDebug_ = false;
};

