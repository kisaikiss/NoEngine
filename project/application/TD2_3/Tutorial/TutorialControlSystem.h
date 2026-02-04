#pragma once
#include "engine/NoEngine.h"

class TutorialControlSystem : public No::ISystem
{
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	float timer_ = 0.0f;
	const float kRoundDuration_ = 3.0f;
	const float kBallPongDuration_ = 3.0f;
};
