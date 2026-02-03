#pragma once
#include "engine/NoEngine.h"

class GameOverSystem : public No::ISystem
{
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:

	float time_ = 0.f;
};