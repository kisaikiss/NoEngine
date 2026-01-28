#pragma once
#include "engine/NoEngine.h"

class VausControlSystem : public No::ISystem
{
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	float CalculateMouseAngle();
private:
	bool wasPress_ = false;
	bool isPress_ = false;
};

