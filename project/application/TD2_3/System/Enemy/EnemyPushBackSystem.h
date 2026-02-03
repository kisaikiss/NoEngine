#pragma once
#include "engine/NoEngine.h"

class EnemyPushBackSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	void UpdateCollider(No::Registry& registry);

	bool CheckSphereToSphere(const NoEngine::Vector3& center1, const NoEngine::Vector3& center2, const float radius1, const float radius2);

	void PushBack(No::Registry& registry, No::Entity e1, No::Entity e2);
};

