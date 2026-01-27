#pragma once
#include "engine/NoEngine.h"

class BackGroundEffectSystem : public No::ISystem
{
public:
	void Update(No::Registry& registry, float deltaTime) override;

};
