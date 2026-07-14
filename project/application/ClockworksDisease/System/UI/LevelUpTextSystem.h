#pragma once
#include "engine/NoEngine.h"



struct CloseLevelUpUITag { float t = 0.0f; };


class LevelUpTextSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:

};