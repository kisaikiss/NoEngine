#pragma once
#include "engine/NoEngine.h"

class TestSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	float angle_ = 0.f;
};

