#pragma once
#include "engine/NoEngine.h"

class RootVegetableGenerateSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	float timer_ = 0.f;
	float generateTime_ = 10.f;

	bool isCarrot_ = false;

	No::Vector3 GenerateRandomPointInCircle(float minRadius, float maxRadius);
};

