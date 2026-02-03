#pragma once
#include "engine/NoEngine.h"
#include "application/TD2_3/Loader/EnemyResources.h"

class BatGenerateSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	float timer_ = 0.f;
	bool isGreen_ = false;

	EnemyResources resource_;

	NoEngine::Vector3 GenerateRandomPointInCircle(float minRadius, float maxRadius);
};

