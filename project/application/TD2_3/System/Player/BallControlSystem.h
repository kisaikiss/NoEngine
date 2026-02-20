#pragma once
#include "engine/NoEngine.h"

class BallControlSystem : public No::ISystem
{
public:
    void Update(No::Registry& registry, float deltaTime) override;
private:
	int32_t ballCount_ = 0;
};